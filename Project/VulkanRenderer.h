// @VulkanRenderer.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include "RendererApi.h"

namespace base_engine {

class VulkanRenderer : public RendererApi {
 public:
  void Init() override;
  void Shutdown() override;

  void BeginFrame() override;
  void EndFrame() override;
  void BeginRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<RenderPass> renderPass,
	  bool explicitClear) override;
  void EndRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer) override;
  void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material,
	  Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, const glm::mat4& transform,
	  uint32_t indexCount) override;

private:
};
}  // namespace base_engine