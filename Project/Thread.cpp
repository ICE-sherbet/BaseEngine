#include "Thread.h"
#define NOMINMAX
#include <Windows.h>

#include "Assert.h"
#include "Log.h"

namespace base_engine {
Thread::Thread(const std::string& name) : name_(name) {}

void Thread::SetName(const std::string& name) {
  const HANDLE thread_handle = thread_.native_handle();
  name_ = name;
  const std::wstring wName(name.begin(), name.end());
  if (HRESULT hr = SetThreadDescription(thread_handle, wName.c_str())) {
    BE_CORE_ASSERT(false, "Error: {0}.Failed to set thread name.",
                   std::system_category().message(hr));
  }
  SetThreadAffinityMask(thread_handle, 8);
}

void Thread::Join() { thread_.join(); }
}  // namespace base_engine
