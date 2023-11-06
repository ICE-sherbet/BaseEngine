#include "Window.h"

#include <GLFW/glfw3.h>

#include "../../BaseEngineCore.h"
#include "..\..\BaseEngineRenderer.h"
#include "../../VulkanContext.h"

namespace base_engine {
static bool s_GLFWInitialized = false;

Window::Window(const WindowSpecification& specification)
    : specification_(specification) {}

Window::~Window() { glfwDestroyWindow(window_); }

void Window::Init() {
  data_.Title = specification_.Title;
  data_.Width = specification_.Width;
  data_.Height = specification_.Height;

  if (!s_GLFWInitialized) {
    int success = glfwInit();

    if (!success) {
      __debugbreak();
    }
    s_GLFWInitialized = true;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  if (specification_.Fullscreen) {
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_DECORATED, false);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    window_ = glfwCreateWindow(mode->width, mode->height, data_.Title.c_str(),
                               primaryMonitor, nullptr);
  } else {
    window_ =
        glfwCreateWindow((int)specification_.Width, (int)specification_.Height,
                         data_.Title.c_str(), nullptr, nullptr);
  }

  BASE_ENGINE(RendererContext)->Init();
  renderer_context_ = BASE_ENGINE(RendererContext)->GetRendererContext();
  Ref<VulkanContext> context = renderer_context_.As<VulkanContext>();
  swap_chain_.Init(VulkanContext::GetVkInstance(), context->GetDevice());
  swap_chain_.InitSurface(window_);
  swap_chain_.Create(window_, false);
  
}

void Window::Update() { glfwPollEvents(); }

bool Window::IsShow() { return glfwWindowShouldClose(window_) == GLFW_FALSE; }
}  // namespace base_engine
