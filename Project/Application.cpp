#include "Application.h"
#include "VulkanShaderCompiler.h"

#include <vulkan/vulkan.h>

#include "IWindow.h"
#include "Platform/Windows/Window.h"
#include "RendererApi.h"
#include "VulkanImGuiLayer.h"
#include "VulkanSwapChain.h"

namespace base_engine {
Application* Application::instance_ = nullptr;

namespace {
VulkanSwapChain GetSwapChain() {
  return static_cast<Window&>(Application::Get().GetWindow()).GetSwapChain();
}
}  // namespace

Application::Application(const ApplicationSpecification& spec)
    : render_thread_(spec.threading_policy) {
  instance_ = this;
  Renderer::Init();

  window_ = std::unique_ptr<IWindow>(base_engine::IWindow::Create());
  window_->Init();
}

void Application::Run() {
  is_running_ = true;
  VulkanImGuiLayer layer;
  layer.Init();
  std::filesystem::current_path("../Resource");

  auto shader =
      VulkanShaderCompiler::Compile("Renderer2D.glsl", false, false);

  while (is_running_) {
    render_thread_.BlockUntilRenderComplete();

    render_thread_.Kick();
    window_->Update();

    Renderer::Submit([&]() { GetSwapChain().BeginFrame(); });

    Application* app = this;

    Renderer::Submit([&layer]() { layer.Begin(); });
    Renderer::Submit([&layer]() { layer.End(); });

    Renderer::Submit([&]() { GetSwapChain().Present(); });
  }
}
}  // namespace base_engine
