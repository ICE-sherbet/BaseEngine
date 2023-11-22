#include "UniformBuffer.h"

#include "RendererApi.h"
#include "VulkanUniformBuffer.h"

namespace base_engine {
Ref<UniformBuffer> UniformBuffer::Create(uint32_t size) {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return Ref<VulkanUniformBuffer>::Create(size);
    default:
      return nullptr;
  }
}
}  // namespace base_engine
