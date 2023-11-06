#include "RenderThread.h"

#include <Windows.h>

#include "RendererApi.h"

namespace base_engine {

struct RenderThreadData {
  CRITICAL_SECTION m_CriticalSection;
  CONDITION_VARIABLE m_ConditionVariable;

  RenderThread::State m_State = RenderThread::State::kIdle;
};

RenderThread::RenderThread(const ThreadingPolicy core_threading_policy)
    : threading_policy_(core_threading_policy),
      render_thread_("Render Thread") {
  data_ = new RenderThreadData();

  if (threading_policy_ == ThreadingPolicy::kMultiThreaded) {
    InitializeCriticalSection(&data_->m_CriticalSection);
    InitializeConditionVariable(&data_->m_ConditionVariable);
  }
}

RenderThread::~RenderThread() {
  if (threading_policy_ == ThreadingPolicy::kMultiThreaded)
    DeleteCriticalSection(&data_->m_CriticalSection);
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

  EnterCriticalSection(&data_->m_CriticalSection);
  while (data_->m_State != wait_for_state) {
    SleepConditionVariableCS(&data_->m_ConditionVariable,
                             &data_->m_CriticalSection, INFINITE);
  }
  LeaveCriticalSection(&data_->m_CriticalSection);
}

void RenderThread::WaitAndSet(const State wait_for_state,
                              const State set_to_state) const {
  if (threading_policy_ == ThreadingPolicy::kSingleThreaded) return;

  EnterCriticalSection(&data_->m_CriticalSection);
  while (data_->m_State != wait_for_state) {
    SleepConditionVariableCS(&data_->m_ConditionVariable,
                             &data_->m_CriticalSection, INFINITE);
  }
  data_->m_State = set_to_state;
  WakeAllConditionVariable(&data_->m_ConditionVariable);
  LeaveCriticalSection(&data_->m_CriticalSection);
}

void RenderThread::Set(const State set_to_state) const {
  if (threading_policy_ == ThreadingPolicy::kSingleThreaded) return;

  EnterCriticalSection(&data_->m_CriticalSection);
  data_->m_State = set_to_state;
  WakeAllConditionVariable(&data_->m_ConditionVariable);
  LeaveCriticalSection(&data_->m_CriticalSection);
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
