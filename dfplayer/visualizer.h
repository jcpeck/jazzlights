// Copyright 2014, Igor Chernyshev.
// Licensed under Apache 2.0 ASF license.
//
// Visualization module.

#ifndef __DFPLAYER_VISUALIZER_H
#define __DFPLAYER_VISUALIZER_H

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <GL/glx.h>
#include <pthread.h>
#include <stdint.h>

#include <deque>
#include <vector>

#include "input_alsa.h"
#include "../projectm/src/libprojectM/projectM.hpp"

// Contains array of bytes and deallocates in destructor.
struct Bytes {
  Bytes(void* data, int len);
  ~Bytes();

  void SetData(void* data, int len);
  uint8_t* GetData() const { return data_; }
  int GetLen() const { return len_; }

 private:
  Bytes(const Bytes& src);
  Bytes& operator=(const Bytes& rhs);
  void Clear();
  uint8_t* data_;
  int len_;
};

class Visualizer {
 public:
  Visualizer(int width, int height, int fps);
  ~Visualizer();

  void StartMessageLoop();

  void UseAlsa(const std::string& spec);

  std::vector<std::string> GetPresetNames();
  void UsePreset(const std::string& spec);

  bool GetAndClearImage(Bytes* bytes);

  int GetAndClearOverrunCount();
  int GetAndClearDroppedImageCount();

 private:
  Visualizer(const Visualizer& src);
  Visualizer& operator=(const Visualizer& rhs);

  class WorkItem {
   public:
    WorkItem() {}
    virtual ~WorkItem() {}

    virtual void Run() = 0;
  };

  void Run();
  static void* ThreadEntry(void* arg);

  void CloseInputLocked();
  bool TransferPcmDataLocked();
  void RenderFrameLocked();

  void CreateRenderContext();
  void DestroyRenderContext();

  int width_;
  int height_;
  std::string alsa_device_;
  AlsaInputHandle* alsa_handle_;
  projectM* projectm_;
  int16_t* pcm_buffer_;
  int total_overrun_count_;
  uint8_t* image_buffer_;
  uint32_t image_buffer_size_;
  bool has_image_;
  int dropped_image_count_;
  uint64_t last_render_time_;
  uint32_t ms_per_frame_;
  bool is_shutting_down_;
  bool has_started_thread_;
  std::deque<WorkItem*> work_items_;
  pthread_mutex_t lock_;
  pthread_t thread_;

  // GLX data, used by the worker thread only.
  Display* display_;
  GLXContext gl_context_;
  GLXPbuffer pbuffer_;
};

#endif  // __DFPLAYER_VISUALIZER_H

