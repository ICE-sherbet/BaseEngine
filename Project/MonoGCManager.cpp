#include "MonoGCManager.h"

#pragma comment(lib, "mono-2.0-sgen.lib")
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/object.h>
#include <mono/metadata/profiler.h>

#include <ranges>
#include <unordered_map>

namespace base_engine {
using ReferenceMap = std::unordered_map<GCHandle, MonoObject*>;
struct GCState {
  ReferenceMap StrongReferences;
  ReferenceMap WeakReferences;
};

static GCState* s_GCState = nullptr;
void MonoGCManager::Init() { s_GCState = new GCState(); }

void MonoGCManager::Shutdown() {
  if (!s_GCState->StrongReferences.empty()) {
    for (const auto handle : s_GCState->StrongReferences | std::views::keys)
      mono_gchandle_free_v2(handle.value);

    s_GCState->StrongReferences.clear();
  }

  if (!s_GCState->WeakReferences.empty()) {
    for (const auto handle : s_GCState->WeakReferences | std::views::keys)
      mono_gchandle_free_v2(handle.value);

    s_GCState->WeakReferences.clear();
  }

  mono_gc_collect(mono_gc_max_generation());
  while (mono_gc_pending_finalizers()) {
  };
  delete s_GCState;
  s_GCState = nullptr;
}

void MonoGCManager::CollectGarbage(bool block_until_finalized) {
  mono_gc_collect(mono_gc_max_generation());
  if (block_until_finalized) {
    while (mono_gc_pending_finalizers())
      ;
  }
}

GCHandle MonoGCManager::CreateObjectReference(MonoObject* managedObject,
                                              bool weakReference, bool pinned,
                                              bool track) {
  GCHandle handle = weakReference
                        ? mono_gchandle_new_weakref_v2(managedObject, pinned)
                        : mono_gchandle_new_v2(managedObject, pinned);
  handle.value = weakReference
                     ? mono_gchandle_new_weakref_v2(managedObject, pinned)
                     : mono_gchandle_new_v2(managedObject, pinned);
  if (track) {
    if (weakReference)
      s_GCState->WeakReferences[handle] = managedObject;
    else
      s_GCState->StrongReferences[handle] = managedObject;
  }

  return handle;
}

bool MonoGCManager::IsHandleValid(const GCHandle handle) {
  if (handle == nullptr) return false;

  MonoObject* obj = mono_gchandle_get_target_v2(handle.value);

  if (obj == nullptr) return false;

  if (mono_object_get_vtable(obj) == nullptr) return false;

  return true;
}

MonoObject* MonoGCManager::GetReferencedObject(GCHandle handle) {
  MonoObject* obj = mono_gchandle_get_target_v2(handle.value);
  if (obj == nullptr || mono_object_get_vtable(obj) == nullptr) return nullptr;
  return obj;
}

void MonoGCManager::ReleaseObjectReference(const GCHandle handle) {
  if (mono_gchandle_get_target_v2(handle.value) != nullptr) {
    mono_gchandle_free_v2(handle.value);
  } else {
    return;
  }

  if (s_GCState->StrongReferences.contains(handle))
    s_GCState->StrongReferences.erase(handle);

  if (!s_GCState->WeakReferences.contains(handle))
    s_GCState->WeakReferences.erase(handle);
}
}  // namespace base_engine
