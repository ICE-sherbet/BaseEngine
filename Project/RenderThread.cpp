#include "RenderThread.h"

#define NOMINMAX

#include <Windows.h>

#include "RendererApi.h"

namespace base_engine {

struct RenderThreadData {
  CRITICAL_SECTION critical_section;
  CONDITION_VARIABLE condition_variable;

  RenderThread::State state = RenderThread::State::kIdle;
};

RenderThread::RenderThread(const ThreadingPolicy core_threading_policy)
    : threading_policy_(core_threading_policy),
      render_thread_("Render Thread") {
  data_ = new RenderThreadData();

  if (threading_policy_ == ThreadingPolicy::kMultiThreaded) {
    InitializeCriticalSection(&data_->critical_section);
    InitializeConditionVariable(&data_->condition_variable);
  }
}

RenderThread::~RenderThread() {
  if (threading_policy_ == ThreadingPolicy::kMultiThreaded)
    DeleteCriticalSection(&data_->critical_section);
}

void RenderThread::Run() {
  is_running_ = true;
  if (threading_policy_ == ThreadingPolicy::kMultiThreaded)
    render_thread_.Dispatch(Renderer::RenderThreadFunc, this);
}

void RenderThread::Terminate() {
  is_running_ = false;
  Pump();

  if (threading_policy_ == ThreadingPolicy::kMultiThreaded)
    render_thread_.Join();
}

void RenderThread::Wait(const State wait_for_state) const {
  if (threading_policy_ == ThreadingPolicy::kSingleThreaded) return;

  EnterCriticalSection(&data_->critical_section);
  while (data_->state != wait_for_state) {
    SleepConditionVariableCS(&data_->condition_variable,
                             &data_->critical_section, INFINITE);
  }
  LeaveCriticalSection(&data_->critical_section);
}

void RenderThread::WaitAndSet(const State wait_for_state,
                              const State set_to_state) const {
  if (threading_policy_ == ThreadingPolicy::kSingleThreaded) return;

  EnterCriticalSection(&data_->critical_section);
  while (data_->state != wait_for_state) {
    SleepConditionVariableCS(&data_->condition_variable,
                             &data_->critical_section, INFINITE);
  }
  data_->state = set_to_state;
  WakeAllConditionVariable(&data_->condition_variable);
  LeaveCriticalSection(&data_->critical_section);
}

void RenderThread::Set(const State set_to_state) const {
  if (threading_policy_ == ThreadingPolicy::kSingleThreaded) return;

  EnterCriticalSection(&data_->critical_section);
  data_->state = set_to_state;
  WakeAllConditionVariable(&data_->condition_variable);
  LeaveCriticalSection(&data_->critical_section);
}

void RenderThread::NextFrame() {
  ++app_thread_frame_;
  Renderer::SwapQueues();
}

void RenderThread::BlockUntilRenderComplete() const {
  if (threading_policy_ == ThreadingPolicy::kSingleThreaded) return;

  Wait(State::kIdle);
}

void RenderThread::Kick() {
  if (threading_policy_ == ThreadingPolicy::kMultiThreaded) {
    Set(State::kKick);
  } else {
    Renderer::WaitAndRender(this);
  }
}

void RenderThread::Pump() {
  NextFrame();
  Kick();
  BlockUntilRenderComplete();
}
}  // namespace base_engine
