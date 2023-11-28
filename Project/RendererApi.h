// @RendererAPI.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include <glm/glm.hpp>

#include "IndexBuffer.h"
#include "Material.h"
#include "RenderCommandBuffer.h"
#include "RenderPass.h"
#include "RenderThread.h"
#include "Renderer2D.h"
#include "RendererSubmit.h"
#include "RendererTexture.h"

namespace base_engine {

enum class RendererApiType { kNone, kVulkan };

class RendererApi {
 public:
  virtual void Init() = 0;
  virtual void Shutdown() = 0;

  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;

  virtual void BeginRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer,
                               Ref<RenderPass> renderPass,
                               bool explicitClear = false) = 0;
  virtual void EndRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer) = 0;
  virtual void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer,
                              Ref<Pipeline> pipeline, Ref<Material> material,
                              Ref<VertexBuffer> vertexBuffer,
                              Ref<IndexBuffer> indexBuffer,
                              const glm::mat4& transform,
                              uint32_t indexCount = 0) = 0;

  static constexpr RendererApiType Current() { return current_renderer_api_; }

 private:
  static constexpr RendererApiType current_renderer_api_ =
      RendererApiType::kVulkan;
};

class Renderer {
 public:
  static void Init();
  static void Shutdown();

  static void BeginFrame();
  static void EndFrame();

  template <typename FuncT>
  static void Submit(FuncT&& func) {
    submit_->Submit(std::forward<FuncT>(func));
  }

  template <typename FuncT>
  static void SubmitResourceFree(FuncT&& func) {
    submit_->SubmitResourceFree(std::forward<FuncT>(func));
  }
  static void RenderThreadFunc(RenderThread* render_thread);
  static void WaitAndRender(RenderThread* render_thread);
  static void SwapQueues();

  static void BeginRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer,
                              Ref<RenderPass> renderPass,
                              bool explicitClear = false);
  static void EndRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer);

  static uint32_t RT_GetCurrentFrameIndex();
  static RenderCommandQueue& GetRenderResourceReleaseQueue(uint32_t index);
  static uint32_t GetCurrentFrameIndex();

  Ref<RendererTexture2D> static GetWhiteTexture();

  static void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer,
                             Ref<Pipeline> pipeline, Ref<Material> material,
                             Ref<VertexBuffer> vertexBuffer,
                             Ref<IndexBuffer> indexBuffer,
                             const glm::mat4& transform,
                             uint32_t indexCount = 0);

 private:
  static inline RendererSubmit* submit_ = nullptr;
};

}  // namespace base_engine
