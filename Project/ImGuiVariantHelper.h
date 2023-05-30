// @ImGuiVariantHelper.h
// @brief
// @author ICE
// @date 2023/05/24
//
// @details

#pragma once
#include <functional>
#include <string>

#include "ScriptTypes.h"
#include "imgui/imgui.h"

namespace base_engine::editor::ImGuiHelper {
bool VariantField(const std::string& label, Variant* variant);
}  // namespace base_engine::editor::ImGuiHelper
