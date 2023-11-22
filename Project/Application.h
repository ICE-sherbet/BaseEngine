// @Application.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include "IWindow.h"
#include "RenderThread.h"

namespace base_engine {

struct ApplicationSpecification {
  uint32_t width;
  uint32_t height;
  std::string title;

  ThreadingPolicy threading_policy = ThreadingPolicy::kSingleThreaded;
};

class Application {
 public:
  IWindow& GetWindow() { return *window_; }

  static Application& Get() { return *instance_; }

  Application(const ApplicationSpecification& spec);
  void Run();

  uint32_t GetCurrentFrameIndex() const { return current_frame_index_; }

 private:
  static Application* instance_;

  RenderThread render_thread_;
  bool is_running_ = false;
  std::unique_ptr<IWindow> window_;

  uint32_t current_frame_index_ = 0;
};
}  // namespace base_engine
