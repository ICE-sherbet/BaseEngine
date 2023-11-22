#include "RendererStats.h"

namespace base_engine::renderer_utils {
static ResourceAllocationCounts s_ResourceAllocationCounts;
ResourceAllocationCounts& GetResourceAllocationCounts() {
  return s_ResourceAllocationCounts;
}
}  // namespace base_engine::renderer_utils