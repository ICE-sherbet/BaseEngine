#include "ScriptComponent.h"

#include "CSharpScriptEngine.h"

namespace base_engine::component {
void ScriptComponent::_Bind() {
  ComponentDB::BindMethod("SetScriptClassHandle",
                          &ScriptComponent::SetScriptClassHandle);
  ComponentDB::BindMethod("GetScriptClassHandle",
                          &ScriptComponent::ScriptClassHandle);
  ADD_PROPERTY(PropertyInfo(VariantType::kAssetHandle, "script",
                            PropertyHint::kAsset, "Script"),
               "SetScriptClassHandle", "GetScriptClassHandle");
}

AssetHandle ScriptComponent::ScriptClassHandle() const {
  return script_class_handle;
}

void ScriptComponent::SetScriptClassHandle(
    const AssetHandle& script_class_handle) {
  this->script_class_handle = script_class_handle;
}
}  // namespace base_engine::component
