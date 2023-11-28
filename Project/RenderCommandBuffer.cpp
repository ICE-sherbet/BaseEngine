#include "RenderCommandBuffer.h"

#include "RendererApi.h"
#include "VulkanRenderCommandBuffer.h"

namespace base_engine {
Ref<RenderCommandBuffer> RenderCommandBuffer::Create(
    uint32_t count, const std::string& debugName) {
  switch (RendererApi::Current()) {
    case RendererApiType::kNone:
      return nullptr;
    case RendererApiType::kVulkan:
      return Ref<VulkanRenderCommandBuffer>::Create(count, debugName);
  }
  return nullptr;
}
}  // namespace base_engine
