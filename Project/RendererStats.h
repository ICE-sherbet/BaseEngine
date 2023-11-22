// @RendererStats.h
// @brief
// @author ICE
// @date 2023/11/22
// 
// @details

#pragma once
#include <cstdint>

namespace base_engine::renderer_utils
{
struct ResourceAllocationCounts {
  uint32_t samplers = 0;
};

ResourceAllocationCounts& GetResourceAllocationCounts();
}
