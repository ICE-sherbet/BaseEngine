// @MonoGCManager.h
// @brief
// @author ICE
// @date 2023/04/07
// 
// @details

#pragma once
#include "ScriptTypes.h"
#include "MonoGCHandle.h"

extern "C" {
typedef struct _MonoObject MonoObject;
}

namespace base_engine
{
	
class MonoGCManager
{
public:
  static void Init();
  static void Shutdown();

  static void CollectGarbage(bool block_until_finalized = true);

  static GCHandle CreateObjectReference(MonoObject* managedObject,
                                        bool weakReference, bool pinned = false,
                                        bool track = true);
  static bool IsHandleValid(GCHandle handle);
  static MonoObject* GetReferencedObject(GCHandle handle);
  static void ReleaseObjectReference(GCHandle handle);
};
}  // namespace base_engine
