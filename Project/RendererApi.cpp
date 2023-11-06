#include "RendererApi.h"

#include "Assert.h"
#include "VulkanRenderer.h"

namespace base_engine {
void RendererApi::SetApi(const RendererApiType api) {
  current_renderer_api_ = api;
}

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

void Renderer::BeginFrame()
{
  renderer_api_->BeginFrame();
}

void Renderer::EndFrame()
{
	renderer_api_->EndFrame();
}

void Renderer::RenderThreadFunc(RenderThread* render_thread) {
  while (render_thread->IsRunning()) {
    WaitAndRender(render_thread);
  }
}

void Renderer::WaitAndRender(RenderThread* render_thread) {

  {
    render_thread->WaitAndSet(RenderThread::State::kKick,
                              RenderThread::State::kBusy);
//    performanceTimers.RenderThreadWaitTime = waitTimer.ElapsedMillis();
  }

  submit_->Execute();
  render_thread->Set(RenderThread::State::kIdle);

}

void Renderer::SwapQueues()
{
	submit_->SwapQueues();
}
}  // namespace base_engine
