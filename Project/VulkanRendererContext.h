// @VulkanRendererContext.h
// @brief
// @author ICE
// @date 2023/10/31
//
// @details

#pragma once
#include "BaseEngineRenderer.h"
#include "VulkanContext.h"

namespace base_engine {
class VulkanRendererContext : public IBaseEngineRendererContext {
 public:
  Ref<RendererContext> GetRendererContext() override;

  void Init() override;

private:
  Ref<RendererContext> renderer_context_;
};
}  // namespace base_engine