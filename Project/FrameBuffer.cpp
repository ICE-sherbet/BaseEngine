#include "FrameBuffer.h"

#include "RendererApi.h"
#include "VulkanFrameBuffer.h"

namespace base_engine {
Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec) {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return Ref<VulkanFrameBuffer>::Create(spec);
    default:
      return nullptr;
  }
}
}  // namespace base_engine
