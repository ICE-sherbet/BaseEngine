// @RenderThread.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include <atomic>
#include <cstdint>

#include "Thread.h"

namespace base_engine {

struct RenderThreadData;

enum class ThreadingPolicy {
  kNone = 0,
  kSingleThreaded,
  kMultiThreaded
};

class RenderThread {
 public:
  enum class State { kIdle = 0, kBusy, kKick };

  explicit RenderThread(ThreadingPolicy core_threading_policy);
  ~RenderThread();

  void Run();
  bool IsRunning() const { return is_running_; }
  void Terminate();

  void Wait(State wait_for_state) const;
  void WaitAndSet(State wait_for_state, State set_to_state) const;
  void Set(State set_to_state) const;

  void NextFrame();
  void BlockUntilRenderComplete() const;
  void Kick();

  void Pump();

 private:
  RenderThreadData* data_;
  ThreadingPolicy threading_policy_;

  Thread render_thread_;

  bool is_running_ = false;

  std::atomic<uint32_t> app_thread_frame_ = 0;
};

}  // namespace base_engine
