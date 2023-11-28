#include "RendererApi.h"

#include "Application.h"
#include "Assert.h"
#include "VulkanRenderer.h"
#include "VulkanSwapChain.h"

namespace base_engine {
static Ref<RendererTexture2D> sWhiteTexture;
;
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
  sWhiteTexture =
      RendererTexture2D::Create(TextureSpecification(), "no-texture.png");
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

void Renderer::BeginRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer,
                               Ref<RenderPass> renderPass, bool explicitClear) {
  renderer_api_->BeginRenderPass(renderCommandBuffer, renderPass,
                                 explicitClear);
}

void Renderer::EndRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer)
{
	renderer_api_->EndRenderPass(renderCommandBuffer);
}

uint32_t Renderer::RT_GetCurrentFrameIndex() {
  return VulkanSwapChain::Get()->GetCurrentBufferIndex();
}

RenderCommandQueue& Renderer::GetRenderResourceReleaseQueue(uint32_t index)
{
  return submit_->GetRenderResourceReleaseQueue(index);
}

uint32_t Renderer::GetCurrentFrameIndex() {
  return Application::Get().GetCurrentFrameIndex();
}

Ref<RendererTexture2D> Renderer::GetWhiteTexture() { return sWhiteTexture; }

void Renderer::RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer,
                              Ref<Pipeline> pipeline, Ref<Material> material,
                              Ref<VertexBuffer> vertexBuffer,
                              Ref<IndexBuffer> indexBuffer,
                              const glm::mat4& transform, uint32_t indexCount) {
  renderer_api_->RenderGeometry(renderCommandBuffer, pipeline, material,
                                vertexBuffer, indexBuffer, transform,
                                indexCount);
}
}  // namespace base_engine
