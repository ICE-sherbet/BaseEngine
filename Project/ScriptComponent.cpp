#include "ScriptComponent.h"

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
}  // namespace base_engine::component
