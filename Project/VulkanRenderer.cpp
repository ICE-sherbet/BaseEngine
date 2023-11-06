#include "VulkanRenderer.h"

#include "Application.h"
#include "VulkanContext.h"
#include "Platform/Windows/Window.h"

namespace base_engine {
void VulkanRenderer::Init() {}

void VulkanRenderer::Shutdown() {}

void VulkanRenderer::BeginFrame()
{
  Renderer::Submit([]() {

    VulkanSwapChain& swap_chain = dynamic_cast<Window&>(Application::Get().GetWindow()).GetSwapChain();


  });
}

void VulkanRenderer::EndFrame() {}
}  // namespace base_engine
