#include "RendererContext.h"

#include "VulkanContext.h"

namespace base_engine {

Ref<RendererContext> RendererContext::Create() {
  return Ref<VulkanContext>::Create();
}

}  // namespace base_engine
