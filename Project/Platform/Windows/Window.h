// @Window.h
// @brief
// @author ICE
// @date 2023/10/17
//
// @details

#pragma once
#include <string>

#include "../../../IWindow.h"

struct GLFWwindow;
struct GLFWcursor;

namespace base_engine {
class RendererContext;

struct WindowSpecification {
  std::string Title = "BE";
  uint32_t Width = 1280;
  uint32_t Height = 720;
  bool Decorated = true;
  bool Fullscreen = false;
  bool VSync = true;
};

class Window : public IWindow {
 public:
  explicit Window(
      const WindowSpecification& specification = WindowSpecification());
  ~Window() override;
  void Init() override;
  void Update() override;
  bool IsShow() override;

 private:
  struct GLFWwindow* window_ = nullptr;
  struct GLFWcursor* ImGuiMouseCursors_[9] = {0};
  WindowSpecification specification_;
  struct WindowData {
    std::string Title;
    uint32_t Width, Height;
  };

  WindowData data_;
  float last_frame_time_ = 0.0f;

  Ref<RendererContext> renderer_context_;
};
}  // namespace base_engine
