// @RenderCommandQueue.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include <cstdint>

namespace base_engine {
class RenderCommandQueue {
 public:
  typedef void (*RenderCommandFn)(void*);

  RenderCommandQueue();
  ~RenderCommandQueue();

  void* Allocate(RenderCommandFn func, uint32_t size);

  void Execute();

private:
  uint8_t* command_buffer_;
  uint8_t* command_buffer_ptr_;
  uint32_t command_count_ = 0;
};

}  // namespace base_engine
