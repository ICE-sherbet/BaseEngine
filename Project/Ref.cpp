#include "Ref.h"

#include <mutex>
#include <unordered_set>

#include "Assert.h"

namespace base_engine {

static std::unordered_set<void*> live_references;
static std::mutex live_reference_mutex;

namespace detail {

void AddToLiveReferences(void* instance) {
  std::scoped_lock lock(live_reference_mutex);
  BE_CORE_ASSERT(instance);
  live_references.insert(instance);
}

void RemoveFromLiveReferences(void* instance) {
  std::scoped_lock lock(live_reference_mutex);
  BE_CORE_ASSERT(instance);
  BE_CORE_ASSERT(live_references.contains(instance));
  live_references.erase(instance);
}

bool IsLive(void* instance) {
  BE_CORE_ASSERT(instance);
  return live_references.contains(instance);
}
}  // namespace detail
}  // namespace base_engine
