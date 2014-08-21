#!/usr/bin/python
#
# Start dfplayer.

import argparse
import os
import shutil
import subprocess
import time

_PROJ_DIR = os.path.dirname(__file__)


def main():
  os.chdir(_PROJ_DIR)

  arg_parser = argparse.ArgumentParser(description='Start player')
  arg_parser.add_argument('--gdb', action='store_true')
  arg_parser.add_argument('--no-reset', action='store_true')
  arg_parser.add_argument('--disable-net', action='store_true')
  arg_parser.add_argument('--mpd', action='store_true')
  arg_parser.add_argument('--disable-fin', action='store_true')
  arg_parser.add_argument('--max', action='store_true')
  arg_parser.add_argument('--nosound', action='store_true')
  arg_parser.add_argument('--prod', action='store_true')
  args = arg_parser.parse_args()

  if not args.nosound:
    shutil.copyfile(
        'dfplayer/asoundrc.sample', '/home/' + os.getlogin() + '/.asoundrc')

  params = ['env/bin/dfplayer', '--listen=0.0.0.0:8080']
  if args.no_reset:
    params.append('--no-reset')
  if args.disable_net:
    params.append('--disable-net')
  if args.disable_fin:
    params.append('--disable-fin')
  if args.mpd:
    params.append('--mpd')
  if args.max or args.prod:
    params.append('--max')

  try:
    if args.gdb:
      subprocess.check_call(
          ['gdb', '-ex', 'run', '--args', 'env/bin/python'] + params)
          #['gdb', '--args', 'env/bin/python'] + params)
    else:
      subprocess.check_call(params)
  except KeyboardInterrupt:
    print 'Player is exiting via KeyboardInterrupt'

  if args.prod:
    print 'dfplayer has exited and start.py script is now sleeping'
    time.sleep(3600)

main()

