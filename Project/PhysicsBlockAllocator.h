﻿// @PhysicsBlockAllocator.h
// @brief
// @author ICE
// @date 2022/10/16
//
// @details

#pragma once
#include <cstdint>

namespace base_engine::physics {
constexpr int32_t kBlockSizeCount = 14;

struct PhysicsBlock;
struct PhysicsChunk;

class PhysicsBlockAllocator {
 public:
  PhysicsBlockAllocator();
  ~PhysicsBlockAllocator();

  void* Allocate(int32_t size);

  void Free(void* p, int32_t size);

  void Clear();

private:
  PhysicsChunk* m_chunks;
  int32_t m_chunkCount;
  int32_t m_chunkSpace;

  PhysicsBlock* m_freeLists[kBlockSizeCount];
};
}  // namespace base_engine::physics
