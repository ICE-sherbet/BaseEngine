#include "BaseEngineRenderer.h"

#include "RendererContext.h"
#include "VulkanRendererContext.h"

namespace base_engine {

IBaseEngineRendererContext* IBaseEngineRendererContext::Create()
{
	return new VulkanRendererContext;
}
}  // namespace base_engine