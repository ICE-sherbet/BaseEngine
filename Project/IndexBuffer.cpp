#include "IndexBuffer.h"

#include "RendererApi.h"
#include "VulkanIndexBuffer.h"

namespace base_engine {
Ref<IndexBuffer> IndexBuffer::Create(uint64_t size) {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return Ref<VulkanIndexBuffer>::Create(size);
    default:
      return nullptr;
  }
}

Ref<IndexBuffer> IndexBuffer::Create(void* data, uint64_t size) {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return Ref<VulkanIndexBuffer>::Create(data, size);
    default:
      return nullptr;
  }
}
}  // namespace base_engine
