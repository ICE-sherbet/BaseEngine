// @IndexBuffer.h
// @brief
// @author ICE
// @date 2023/11/27
//
// @details

#pragma once
#include "Ref.h"

namespace base_engine {

class IndexBuffer : public RefCounted {
 public:
  virtual ~IndexBuffer() {}

  virtual uint32_t GetCount() const = 0;

  virtual uint64_t GetSize() const = 0;

  static Ref<IndexBuffer> Create(uint64_t size);
  static Ref<IndexBuffer> Create(void* data, uint64_t size = 0);
};
}  // namespace base_engine
