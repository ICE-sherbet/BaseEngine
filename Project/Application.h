// @Application.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include "IWindow.h"
#include "ImGuiLayer.h"
#include "RenderThread.h"

namespace base_engine {

struct ApplicationSpecification {
  uint32_t width;
  uint32_t height;
  std::string title;

  bool enable_editor;
  ThreadingPolicy threading_policy = ThreadingPolicy::kSingleThreaded;
};

class Application {
 public:
  static Application& Get() { return *instance_; }

  explicit Application(const ApplicationSpecification& spec);

  void Run();

  IWindow& GetWindow() { return *window_; }

 private:
  static Application* instance_;

  RenderThread render_thread_;
  bool is_running_ = false;
  std::unique_ptr<IWindow> window_;
  bool enable_editor_ = false;
  std::unique_ptr<ImGuiLayer> imgui_layer_;
};
}  // namespace base_engine
