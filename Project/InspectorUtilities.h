// @InspectorUtilities.h
// @brief
// @author ICE
// @date 2023/06/27
//
// @details

#pragma once
#include "EditorProperty.h"

namespace base_engine::editor::inspector {

std::shared_ptr<EditorProperty> MakeEditorProperty(
    void* object, VariantType type, PropertyHint hint,
    const std::string& hint_name);
}  // namespace base_engine::editor::inspector
