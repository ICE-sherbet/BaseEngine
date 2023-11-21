// @GPUMemoryStats.h
// @brief
// @author ICE
// @date 2023/11/14
//
// @details

#pragma once
#include <cstdint>

namespace base_engine {
struct GPUMemoryStats {
  uint64_t Used = 0;
  uint64_t TotalAvailable = 0;
  uint64_t AllocationCount = 0;

  uint64_t BufferAllocationSize = 0;
  uint64_t BufferAllocationCount = 0;
};
}  // namespace base_engine
