// @UniformBuffer.h
// @brief
// @author ICE
// @date 2023/11/22
// 
// @details

#pragma once
#include "Ref.h"

namespace base_engine
{
class UniformBuffer : public RefCounted
{
public:
  virtual ~UniformBuffer() {}
  virtual void SetData(const void* data, uint32_t size,
                       uint32_t offset = 0) = 0;
  virtual void RT_SetData(const void* data, uint32_t size,
                          uint32_t offset = 0) = 0;

  static Ref<UniformBuffer> Create(uint32_t size);
};
}
