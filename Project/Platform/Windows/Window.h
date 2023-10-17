// @Window.h
// @brief
// @author ICE
// @date 2023/10/17
//
// @details

#pragma once
#include <GLFW/glfw3.h>

#include "IWindow.h"
namespace base_engine {
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

  void Init() override;
  void Update() override;

 private:
  GLFWwindow* m_Window = nullptr;
  GLFWcursor* m_ImGuiMouseCursors[9] = {0};
  WindowSpecification m_Specification;
  struct WindowData {
    std::string Title;
    uint32_t Width, Height;
  };

  WindowData m_Data;
  float m_LastFrameTime = 0.0f;
};
}  // namespace base_engine
