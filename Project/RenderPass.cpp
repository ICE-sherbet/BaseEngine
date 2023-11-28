#include "RenderPass.h"

#include "RendererApi.h"
#include "VulkanRenderPass.h"

namespace base_engine {
Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec) {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return Ref<VulkanRenderPass>::Create(spec);
    default:
      return nullptr;
  }
}
}  // namespace base_engine