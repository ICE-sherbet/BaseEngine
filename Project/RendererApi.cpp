#include "RendererApi.h"

#include "Application.h"
#include "Assert.h"
#include "VulkanRenderer.h"
#include "VulkanSwapChain.h"

namespace base_engine {

static RendererApi* InitRendererAPI() {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return new VulkanRenderer();
  }

  BE_CORE_ASSERT(false, "Unknown RendererAPI!");
  return nullptr;
}

static RendererApi* renderer_api_ = nullptr;

void Renderer::Init() {
  submit_ = new RendererSubmit;
  renderer_api_ = InitRendererAPI();

  renderer_api_->Init();
}

void Renderer::Shutdown() {
  renderer_api_->Shutdown();

  delete submit_;
  submit_ = nullptr;
}

void Renderer::BeginFrame() { renderer_api_->BeginFrame(); }

void Renderer::EndFrame() { renderer_api_->EndFrame(); }

void Renderer::RenderThreadFunc(RenderThread* render_thread) {
  while (render_thread->IsRunning()) {
    WaitAndRender(render_thread);
  }
}

void Renderer::WaitAndRender(RenderThread* render_thread) {
  {
    render_thread->WaitAndSet(RenderThread::State::kKick,
                              RenderThread::State::kBusy);
  }

  submit_->Execute();
  render_thread->Set(RenderThread::State::kIdle);
}

void Renderer::SwapQueues() { submit_->SwapQueues(); }

uint32_t Renderer::RT_GetCurrentFrameIndex() {
  return VulkanSwapChain::Get()->GetCurrentBufferIndex();
}

uint32_t Renderer::GetCurrentFrameIndex() {
  return Application::Get().GetCurrentFrameIndex();
}
}  // namespace base_engine
