#include "RenderCommandQueue.h"

#include <cstring>

namespace base_engine {
RenderCommandQueue::RenderCommandQueue() {
  constexpr size_t size = 10 * 1024 * 1024;
  command_buffer_ = new uint8_t[size];
  command_buffer_ptr_ = command_buffer_;
  memset(command_buffer_, 0, size);
}

RenderCommandQueue::~RenderCommandQueue() { delete[] command_buffer_; }

void* RenderCommandQueue::Allocate(const RenderCommandFn func,
                                   const uint32_t size) {
  *reinterpret_cast<RenderCommandFn*>(command_buffer_ptr_) = func;
  command_buffer_ptr_ += sizeof(RenderCommandFn);

  *reinterpret_cast<uint32_t*>(command_buffer_ptr_) = size;
  command_buffer_ptr_ += sizeof(uint32_t);

  void* memory = command_buffer_ptr_;
  command_buffer_ptr_ += size;

  command_count_++;
  return memory;
}

void RenderCommandQueue::Execute() {
  uint8_t* buffer = command_buffer_;

  for (uint32_t i = 0; i < command_count_; i++) {
    const RenderCommandFn function =
        *reinterpret_cast<RenderCommandFn*>(buffer);
    buffer += sizeof(RenderCommandFn);

    const uint32_t size = *reinterpret_cast<uint32_t*>(buffer);
    buffer += sizeof(uint32_t);
    function(buffer);
    buffer += size;
  }

  command_buffer_ptr_ = command_buffer_;
  command_count_ = 0;
}
}  // namespace base_engine
