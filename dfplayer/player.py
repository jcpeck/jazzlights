# -*- coding: utf-8 -*-

import atexit
import gevent
import logging
import math
import mpd
import os
import PIL
import re
import signal
import StringIO
import subprocess
import sys
import time

from PIL import Image
from PIL import ImageChops
from gevent import sleep
from gevent.coros import RLock

from .frame_source import FrameSource
from .stats import Stats
from .util import catch_and_log, PROJECT_DIR, PACKAGE_DIR, VENV_DIR
from .util import get_time_millis
from .tcl_renderer import TclRenderer
from .renderer_cc import Visualizer

FPS = 30
_SCREEN_FRAME_WIDTH = 500
IMAGE_FRAME_WIDTH = _SCREEN_FRAME_WIDTH / 2
FRAME_HEIGHT = 50
MESH_RATIO = 5  # Make it 50x10
TCL_CONTROLLER = 1
_USE_CC_TCL = True

MPD_PORT = 6605
MPD_DIR = VENV_DIR + '/mpd'
MPD_CONFIG_FILE = MPD_DIR + '/mpd.conf'
MPD_DB_FILE = MPD_DIR + '/tag_cache'
MPD_PID_FILE = MPD_DIR + '/mpd.pid'
MPD_LOG_FILE = MPD_DIR + '/mpd.log'
MPD_CARD_ID = -1  # From 'aplay -l'

CLIPS_DIR = VENV_DIR + '/clips'
PLAYLISTS_DIR = VENV_DIR + '/playlists'

_PRESET_DIR = 'projectm/presets'
#_PRESET_DIR = 'projectm/presets_yin'
_PRESET_DURATION = 10000

# See http://manpages.ubuntu.com/manpages/lucid/man5/mpd.conf.5.html
MPD_CONFIG_TPL = '''
music_directory     "%(CLIPS_DIR)s"
playlist_directory  "%(PLAYLISTS_DIR)s"
db_file             "%(MPD_DB_FILE)s"
pid_file            "%(MPD_PID_FILE)s"
log_file            "%(MPD_LOG_FILE)s"
port                "%(MPD_PORT)d"
audio_output {
    type            "alsa"
    name            "DF audio loop"
    device          "df_dup_output"
    auto_resample   "no"
}
'''
#    device          "df_output_mdev"
#    device          "df_real_device"
#    device          "hw:%(MPD_CARD_ID)s,0"
#    mixer_type      "hardware"
#    mixer_device    "hw:%(MPD_CARD_ID)s"


class Player(object):

    def __init__(self, playlist):
        self._update_card_id()

        self._start_mpd()
        self.lock = RLock()
        with self.lock:
            self.mpd = mpd.MPDClient()
            self.mpd.connect('localhost', MPD_PORT)

        self._playlist_name = playlist
        self.playlist = []

        self._target_gamma = 2.4
        self._seek_time = None
        self._frame = None
        self._last_frame_file = ''
        self._frame_delay_stats = Stats(100)
        self._render_durations = Stats(100)
        self._visualization_period_stats = Stats(100)

        self._frame_source = FrameSource(
            FPS, _SCREEN_FRAME_WIDTH, IMAGE_FRAME_WIDTH,
            FRAME_HEIGHT, CLIPS_DIR)

        self._tcl = TclRenderer(
            TCL_CONTROLLER, _SCREEN_FRAME_WIDTH, FRAME_HEIGHT,
            'dfplayer/layout.dxf', self._target_gamma, _USE_CC_TCL)

        self._use_visualization = False
        self._visualizer = None

        self._load_playlist()
        self._fetch_state()

    def __str__(self):
        elapsed_sec = int(self.elapsed_time)
        duration, delays = self._tcl.get_and_clear_frame_delays()
        for d in delays:
            self._frame_delay_stats.add(d)
        if self._visualizer:
            for d in self._visualizer.GetAndClearFramePeriods():
                self._visualization_period_stats.add(d)
        # A rather hacky way to calculate FPS. Depends on get/clear
        # timestamp. OK while we call it once per second though.
        fps = 0
        if duration > 0:
          fps = float(int(10000.0 * len(delays) / duration)) / 10.0
        frame_avg = self._frame_delay_stats.get_average_and_stddev()
        visual_period_avg = \
            self._visualization_period_stats.get_average_and_stddev()
        render_avg = self._render_durations.get_average_and_stddev()
        return ('Player [%s %s %02d:%02d] (fps=%s, delay=%s/%s, '
                'render=%d/%d, vis=%d/%d, cached=%s, queued=%s)') % (
                   self.status, self.clip_name,
                   elapsed_sec / 60, elapsed_sec % 60, fps,
                   int(frame_avg[0]), int(frame_avg[1]),
                   int(render_avg[0]), int(render_avg[1]),
                   int(visual_period_avg[0]), int(visual_period_avg[1]),
                   self._frame_source.get_cache_size(),
                   self._tcl.get_queue_size())

    def get_status_lines(self):
        if self._seek_time:
            elapsed_time = self._seek_time
        else:
            elapsed_time = self.elapsed_time
        elapsed_sec = int(elapsed_time)
        lines = []
        lines.append('%s / %s / %02d:%02d' % (
            self.status.upper(), self.clip_name,
            elapsed_sec / 60, elapsed_sec % 60))
        lines.append('volume = %s, gamma = %s' % (
            self._volume, self._target_gamma))
        if self._use_visualization:
            lines.append(self._visualizer.GetCurrentPresetNameProgress())
        else:
            lines.append('Playing video')
        return lines

    def get_frame_size(self):
        return (_SCREEN_FRAME_WIDTH, FRAME_HEIGHT)

    def get_tcl_coords(self):
        return self._tcl.get_layout_coords()

    def toggle_split_sides(self):
        self._frame_source.toggle_split_sides()

    def gamma_up(self):
        self._target_gamma += 0.1
        print 'Setting gamma to %s' % self._target_gamma
        self._tcl.set_gamma(self._target_gamma)

    def gamma_down(self):
        self._target_gamma -= 0.1
        if self._target_gamma <= 0:
            self._target_gamma = 0.1
        print 'Setting gamma to %s' % self._target_gamma
        self._tcl.set_gamma(self._target_gamma)

    def _fetch_playlist(self, is_startup):
        if len(self.playlist) == self._target_playlist_len:
            return

        # Ideally, we could keep loading more songs over time, but load()
        # seems to duplicate song names in the list, while clear() aborts
        # the current playback.
        # TODO(igorc): See if we can load() ver time without duplicating.
        # Otherwise, adding just 6 (although large) songs takes ~5 seconds.
        listinfo = []
        while True:
            # The playlist gets loaded over some period of time.
            logging.info('Fetching MPD playlist (%s out of %s done)' % (
                len(listinfo), self._target_playlist_len))
            with self.lock:
                self.mpd.clear()
                self.mpd.load(self._playlist_name)
                listinfo = self.mpd.playlistinfo()
                if len(listinfo) >= self._target_playlist_len:
                    break
            if is_startup:
                sleep(1)
            else:
                break

        if len(self.playlist) == len(listinfo):
            return

        self.playlist = []
        self.songid_to_idx = {}
        for song in listinfo:
            self.songid_to_idx[song['id']] = len(self.playlist)
            self.playlist.append((song['file'])[0:-4])

        logging.info('MPD playlist loaded %s songs, we need %s' % (
            len(listinfo), self._target_playlist_len))
        logging.info('Loaded playlist: %s' % (self.playlist))

    def _load_playlist(self):
        with self.lock:
            self._target_playlist_len = len(
                self.mpd.listplaylist(self._playlist_name))
            self.mpd.repeat(1)
            self.mpd.single(1)
            self.mpd.update()
        # Give MPD some time to find music files before loading the playlist.
        # This helps playlist to be more complete on the first load.
        sleep(1)
        with self.lock:
            self.mpd.load(self._playlist_name)
        self._fetch_playlist(True)

    def _read_state(self):
        with self.lock:
            try:
                return self.mpd.status()
            except IOError:
                print 'IOErrror accessing MPD status'
                return None
            except mpd.ConnectionError:
                print 'ConnectionError accessing MPD status'
                return None

    def _fetch_state(self):
        # TODO(igorc): Why do we lock access to mpd, but not the rest of vars?
        # TODO(igorc): Add "mpd" prefix to all mpd-related state vars.
        s = self._read_state()
        if not s:
            return

        self._mpd_state_ts = time.time()
        self._volume = 0.01 * float(s['volume'])
        self._seek_time = None
        if s['state'] == 'play':
            self.status = 'playing'
        elif s['state'] == 'pause':
            self.status = 'paused'
        else:
            self.status = 'idle'
            self.clip_name = None
            self._mpd_elapsed_time = 0

        if 'error' in s:
            # TODO(azov): Report error on the UI.
            logging.info('MPD Error = \'%s\'', s['error'])

        if self.status != 'idle':
            self._songid = s['songid']
            new_clip = self.playlist[self.songid_to_idx[self._songid]]
            self.clip_name = new_clip
            self._mpd_elapsed_time = float(s['elapsed'])

        # print 'MPD status', s

        # self._fetch_playlist(False)

    def _config_mpd(self):
        for d in (MPD_DIR, CLIPS_DIR, PLAYLISTS_DIR):
            if not os.path.exists(d):
                os.makedirs(d)

        with open(MPD_CONFIG_FILE, 'w') as out:
            out.write(MPD_CONFIG_TPL % globals())

    def _update_card_id(self):
        global MPD_CARD_ID
        if MPD_CARD_ID != -1:
            return

        with open('/proc/asound/modules') as f:
            alsa_modules = f.readlines()
        re_term = re.compile("\s*(\d*)\s*snd_usb_audio\s*")
        for line in alsa_modules:
            m = re_term.match(line)
            if m:
                MPD_CARD_ID = int(m.group(1))
                logging.info('Located USB card #%s' % MPD_CARD_ID)
                break
        if MPD_CARD_ID == -1:
            logging.info('Unable to find USB card id')
            MPD_CARD_ID = 1  # Maybe better than nothing

    def _stop_mpd(self):
        self._config_mpd()
        if not os.path.exists(MPD_PID_FILE):
            self._kill_mpd()
            return
        logging.info('Stopping mpd')
        try:
            subprocess.call(['mpd', '--kill', MPD_CONFIG_FILE])
        except:
            logging.info('Error stopping MPD: %s (%s)' % (
                sys.exc_info()[0], sys.exc_info()[1]))
        sleep(1)
        self._kill_mpd()

    def _try_kill_mpd(self, signal_num):
        mpd_pid = None
        pids = [pid for pid in os.listdir('/proc') if pid.isdigit()]
        for pid in pids:
            pid_dir = os.path.join('/proc', pid)
            try:
                exe_link = os.path.join(pid_dir, 'exe')
                #if not bool(os.stat(exe_link).st_mode & stat.S_IRGRP):
                #    continue
                exe = os.readlink(exe_link)
                if os.path.basename(exe) != 'mpd':
                    continue
                with open(os.path.join(pid_dir, 'cmdline'), 'rb') as f:
                    cmdline = f.read().split('\0')
                if len(cmdline) == 3 and cmdline[1] == MPD_CONFIG_FILE:
                    mpd_pid = pid
                    break
                print 'skipped %s / %s' % (exe, len(cmdline))
            except IOError:  # proc has already terminated
                continue
            except OSError:  # no access to that pid
                continue
        if mpd_pid is None:
            return False
        print 'Found live MPD %s, sending signal %s' % (mpd_pid, signal_num)
        os.kill(int(mpd_pid), signal_num)
        return True

    def _kill_mpd(self):
        if self._try_kill_mpd(signal.SIGTERM):
            sleep(1)
        if self._try_kill_mpd(signal.SIGKILL):
            sleep(1)
        if os.path.exists(MPD_PID_FILE):
            os.unlink(MPD_PID_FILE)

    def _start_mpd(self):
        self._config_mpd()
        self._stop_mpd()
        logging.info('Starting mpd')

        if os.path.exists(MPD_DB_FILE):
            os.unlink(MPD_DB_FILE)

        subprocess.check_call(['mpd', MPD_CONFIG_FILE])
        atexit.register(lambda : self._stop_mpd())

    @property
    def elapsed_time(self):
        if self.status == 'playing':
            return time.time() - self._mpd_state_ts + self._mpd_elapsed_time
        else:
            return self._mpd_elapsed_time

    @property
    def volume(self):
        return self._volume

    @volume.setter
    def volume(self, value):
        if value < 0:
            value = 0
        elif value > 1:
            value = 1
        logging.info('Setting volume to %s', value)
        with self.lock:
            self.mpd.setvol(int(float(value) * 100))
            self._volume = value  # Till next status sync.

    def volume_up(self):
        self.volume = self.volume + 0.05

    def volume_down(self):
        self.volume = self.volume - 0.05

    def play(self, clip_idx):
        with self.lock:
            if len(self.playlist) < 1:
                logging.error('Playlist is empty')
            else:
                self.mpd.play(clip_idx % len(self.playlist))

    def toggle(self):
        if self.status == 'idle':
            self.play(0)
        elif self.status == 'playing':
            self.pause()
        else:
            self.resume()

    def pause(self):
        with self.lock:
            self.mpd.pause(1)

    def resume(self):
        with self.lock:
            self.mpd.pause(0)

    def next(self):
        with self.lock:
            self.mpd.next()

    def prev(self):
        with self.lock:
            self.mpd.previous()

    def skip_forward(self):
        self.skip(20)

    def skip_backward(self):
        self.skip(-20)

    def skip(self, seconds):
        if self.status == 'idle':
            return
        if self._seek_time:
            self._seek_time = int(self._seek_time + seconds)
        else:
            self._seek_time = int(self._mpd_elapsed_time + seconds)
        if self._seek_time < 0:
            self._seek_time = 0
        with self.lock:
            # TODO(igorc): Passed None here! (around start/end of track?)
            self.mpd.seekid(self._songid, '%s' % self._seek_time)

    def toggle_visualization(self):
        if not _USE_CC_TCL:
            return
        self._use_visualization = not self._use_visualization
        if not self._visualizer:
            self._visualizer_size = (
                IMAGE_FRAME_WIDTH / MESH_RATIO, FRAME_HEIGHT / MESH_RATIO)
            self._visualizer = Visualizer(
                self._visualizer_size[0], self._visualizer_size[1], 256, FPS,
                _PRESET_DIR, _PRESET_DURATION)
            self._visualizer.StartMessageLoop()
            self._visualizer.UseAlsa('df_dup_input')

    def select_next_preset(self, is_forward):
        if not self._use_visualization:
            return
        if is_forward:
            self._visualizer.SelectNextPreset()
        else:
            self._visualizer.SelectPreviousPreset()

    def get_frame_image(self):
        if self.status == 'idle':
            # TODO(igorc): Keep drawing some neutral pattern for fun.
            return None
        else:
            elapsed_time = self.elapsed_time
            if self._use_visualization:
                start_time = time.time()
                newimg_data = self._visualizer.GetAndClearImage()
                if newimg_data:
                    texsize = self._visualizer.GetTexSize()
                    new_image = Image.fromstring(
                        'RGB', (texsize, texsize), newimg_data)
                    src_img = new_image.resize(
                        (IMAGE_FRAME_WIDTH, FRAME_HEIGHT))
                    frame_img = Image.new(
                        'RGB', (_SCREEN_FRAME_WIDTH, FRAME_HEIGHT))
                    frame_img.paste(src_img, (IMAGE_FRAME_WIDTH, 0))
                    flip_img = src_img.transpose(Image.FLIP_LEFT_RIGHT)
                    frame_img.paste(flip_img, (0, 0))
                    self._frame = frame_img
                duration_ms = int(round((time.time() - start_time) * 1000))
                self._render_durations.add(duration_ms)
                return self._frame

            if self._tcl.has_scheduling_support():
                start_time = time.time()
                self._frame_source.prefetch(self.clip_name, elapsed_time)
                baseline_ms = get_time_millis()
                for f in self._frame_source.get_cached_frames():
                    if not f.rendered:
                        f.rendered = True
                        self._tcl.send_frame(f.image, f.current_ms - baseline_ms)
                new_frame = self._frame_source.get_frame_at(elapsed_time)
                if new_frame:
                    self._frame = new_frame.image
                duration_ms = int(round((time.time() - start_time) * 1000))
                self._render_durations.add(duration_ms)
                return self._frame

            frame_num = int(elapsed_time * FPS)
            frame_file = CLIPS_DIR + '/%s/frame%06d.jpg' \
                % (self.clip_name, frame_num + 1)
            if frame_file == self._last_frame_file:
                return self._frame

            start_time = time.time()
            new_frame = self._frame_source.load_frame_file(
                self.clip_name, frame_num)
            if not new_frame:
                return self._frame
            self._frame = new_frame
            self._last_frame_file = frame_file

            delay_ms = int((float(frame_num) / FPS - elapsed_time) * 1000.0)
            self._tcl.send_frame(self._frame, delay_ms)

            duration_ms = int(round((time.time() - start_time) * 1000))
            self._render_durations.add(duration_ms)

            return self._frame

    def play_effect(self, name, **kwargs):
        logging.info("Playing %s: %s", name, kwargs)
        self._frame_source.play_effect(name, **kwargs)

    def stop_effect(self):
        self._frame_source.stop_effect()

    def run(self):
        while True:
            with catch_and_log():
                self._fetch_state()
                logging.info(self)
                # with self.lock:
                #    self.mpd.idle()
                sleep(1)

