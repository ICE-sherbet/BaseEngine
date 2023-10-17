#include "Window.h"

#include <GLFW/glfw3.h>

namespace base_engine {
static bool s_GLFWInitialized = false;

Window::Window(const WindowSpecification& specification)
    : m_Specification(specification) {}

void Window::Init() {
  m_Data.Title = m_Specification.Title;
  m_Data.Width = m_Specification.Width;
  m_Data.Height = m_Specification.Height;

  if (!s_GLFWInitialized) {
    int success = glfwInit();

    if (!success) {
      __debugbreak();
    }
    s_GLFWInitialized = true;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  if (m_Specification.Fullscreen) {
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    glfwWindowHint(GLFW_DECORATED, false);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    m_Window = glfwCreateWindow(mode->width, mode->height, m_Data.Title.c_str(),
                                primaryMonitor, nullptr);
  } else {
    m_Window = glfwCreateWindow((int)m_Specification.Width,
                                (int)m_Specification.Height,
                                m_Data.Title.c_str(), nullptr, nullptr);
  }
}

void Window::Update() { glfwPollEvents(); }

}  // namespace base_engine
