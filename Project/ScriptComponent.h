// @ScriptComponent.h
// @brief
// @author ICE
// @date 2023/04/07
// 
// @details

#pragma once
#include "Asset.h"
#include "MonoGCHandle.h"

#include "ComponentDB.h"
#include "ComponentProperty.h"

namespace base_engine::component
{
struct ScriptComponent {
  BE_COMPONENT(ScriptComponent)
  AssetHandle script_class_handle;

  std::vector<uint32_t> field_ids;

  GCHandle managed_instance = nullptr;

  bool is_runtime_initialized = false;

  ScriptComponent() = default;
  ScriptComponent(const ScriptComponent& other) = default;
  ScriptComponent(AssetHandle scriptClassHandle)
      : script_class_handle(scriptClassHandle) {}

  static void _Bind();
};
}
