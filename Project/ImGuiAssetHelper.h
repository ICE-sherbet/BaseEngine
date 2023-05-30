// @ImGuiAssetHelper.h
// @brief
// @author ICE
// @date 2023/05/24
//
// @details

#pragma once
#include <string>

#include "Asset.h"
#include "imgui.h"

namespace base_engine::editor::ImGuiHelper {

bool AssetReferenceField(const std::string& label, AssetHandle* asset_handle,
                         AssetType asset_type);
}  // namespace base_engine::editor::ImGuiHelper
