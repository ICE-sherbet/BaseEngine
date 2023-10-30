#include "VulkanRendererContext.h"

namespace base_engine {
Ref<RendererContext> VulkanRendererContext::GetRendererContext() {
  return renderer_context_;
}

void VulkanRendererContext::Init() {
  renderer_context_ = VulkanContext::Create();
  renderer_context_->Init();
}
}  // namespace base_engine
