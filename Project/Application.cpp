#include "Application.h"

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
    : render_thread_(spec.threading_policy),
      enable_editor_(spec.enable_editor) {
  instance_ = this;
  Renderer::Init();

  window_ = std::unique_ptr<IWindow>(base_engine::IWindow::Create());
  window_->Init();

  if (enable_editor_) {
    imgui_layer_ = std::make_unique<VulkanImGuiLayer>();
    imgui_layer_->Init();
  }
}

void Application::Run() {
  is_running_ = true;

  while (is_running_) {
    render_thread_.BlockUntilRenderComplete();

    render_thread_.Kick();
    window_->Update();

    Renderer::Submit([&]() { GetSwapChain().BeginFrame(); });

    if (enable_editor_) {
      Renderer::Submit([&]() { imgui_layer_->Begin(); });
      Renderer::Submit([&]() { imgui_layer_->End(); });
    }

    Renderer::Submit([&]() { GetSwapChain().Present(); });
  }
}
}  // namespace base_engine
