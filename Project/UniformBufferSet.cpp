#include "UniformBufferSet.h"

#include "RendererApi.h"
#include "VulkanUniformBufferSet.h"

namespace base_engine {
Ref<UniformBufferSet> UniformBufferSet::Create(uint32_t size,
                                               uint32_t framesInFlight) {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return Ref<VulkanUniformBufferSet>::Create(size, framesInFlight);
    default:
      return nullptr;
  }
}
}  // namespace base_engine
