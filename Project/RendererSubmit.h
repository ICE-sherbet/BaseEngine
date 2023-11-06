// @RendererSubmit.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include <array>
#include <atomic>

#include "Ref.h"
#include "RenderCommandQueue.h"

namespace base_engine {
constexpr uint32_t kRenderCommandQueueCount = 2;

class RendererSubmit {
 public:
  RendererSubmit();

  ~RendererSubmit();

  template <typename FuncT>
  void Submit(FuncT&& func) {
    auto render_cmd = [](void* ptr) {
      auto pFunc = (FuncT*)ptr;
      (*pFunc)();
      pFunc->~FuncT();
    };
    auto storage_buffer =
        GetRenderCommandQueue().Allocate(render_cmd, sizeof(func));
    new (storage_buffer) FuncT(std::forward<FuncT>(func));
  }

  void Execute();
  void SwapQueues();
 private:
  RenderCommandQueue& GetRenderCommandQueue();

  RenderCommandQueue& GetRenderCommandQueue() const;

private:
  std::atomic<uint32_t> queue_submission_index_ = 0;
  std::array<RenderCommandQueue*, kRenderCommandQueueCount> command_queue_;
};
}  // namespace base_engine
