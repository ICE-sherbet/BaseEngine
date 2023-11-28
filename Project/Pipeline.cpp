#include "Pipeline.h"

#include "RendererApi.h"
#include "VulkanRenderPipeline.h"

namespace base_engine {
Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec) {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return Ref<VulkanRenderPipeline>::Create(spec);
    default:
      return nullptr;
  }
}
}  // namespace base_engine
