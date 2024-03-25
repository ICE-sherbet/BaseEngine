#include "AssetsBrowserItems.h"

#include "AssetsBrowserSetting.h"
#include "EditorTextureResource.h"
#include "RendererApi.h"
#include "SelectManager.h"
#include "VulkanTexture.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "ImGuiUtilities.h"
#include "imgui_internal.h"

namespace base_engine::editor {

AssetsBrowserItem::AssetsBrowserItem(const ItemType type,
                                     const AssetHandle& handle,
                                     const std::string& name,
                                     const Ref<Texture>& icon)
    : type_(type), handle_(handle), icon_(icon), filename_(name) {
  display_name_ = filename_;
  constexpr auto kFileNameMaxLength = 20;
  if (filename_.size() > kFileNameMaxLength) {
    display_name_ = filename_.substr(0, kFileNameMaxLength) + "...";
  }
}

void AssetsBrowserItem::OnRenderBegin() {
  ImGui::PushID(&handle_);
  ImGui::BeginGroup();
}

ImRect RectExpanded(const ImRect& rect, float x, float y) {
  ImRect result = rect;
  result.Min.x -= x;
  result.Min.y -= y;
  result.Max.x += x;
  result.Max.y += y;
  return result;
}
ImRect RectOffset(const ImRect& rect, float x, float y) {
  ImRect result = rect;
  result.Min.x += x;
  result.Min.y += y;
  result.Max.x += x;
  result.Max.y += y;
  return result;
}
AssetsBrowserItemActionResult AssetsBrowserItem::OnRender() {
  ItemActionResult result;

  auto drawShadow = [](const ImVec2& top_left, const ImVec2& bottom_right,
                       const bool directory) {
    auto* draw_list = ImGui::GetWindowDrawList();
    const ImRect item_rect =
        RectOffset(ImRect(top_left, bottom_right), 1.0f, 1.0f);
    draw_list->AddRect(item_rect.Min, item_rect.Max, IM_COL32(15, 15, 15, 255),
                       6.0f, directory ? 0 : ImDrawFlags_RoundCornersBottom,
                       2.0f);
  };
  const float thumbnail_size = AssetsBrowserSetting::Get().thumbnail_size;

  const ImVec2 top_left = ImGui::GetCursorScreenPos();
  const ImVec2 bottom_right = {top_left.x + thumbnail_size,
                               top_left.y + thumbnail_size};

  bool is_selected =
      SelectManager::Instance()->IsSelectItem("AssetsBrowser", handle_);

  ImGui::InvisibleButton("##thumbnailButton",
                         ImVec2{thumbnail_size, thumbnail_size});
  if (ImGui::IsItemHovered()) {
    drawShadow(top_left, bottom_right, true);

    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(top_left, bottom_right, IM_COL32(47, 47, 47, 255),
                             6.0f);
  }
  const auto size = RectExpanded(
      ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()), -6.0f, -6.0f);
  ImGui::GetWindowDrawList()->AddImage(
	  ui::GetTextureID(icon_), size.Min, size.Max, ImVec2(0, 0), ImVec2(1, 1),
      ImGui::IsItemHovered() ? ImColor(255, 255, 255, 255)
                             : ImColor(255, 255, 255, 225));

  ImGui::Text(display_name_.c_str());

  ImGui::EndGroup();

  if (is_selected || ImGui::IsItemHovered()) {
    const auto item_rect =
        ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    auto* draw_list = ImGui::GetWindowDrawList();

    // Outline
    draw_list->AddRect(item_rect.Min, item_rect.Max,
                       ImColor(IM_COL32(237, 192, 119, 255)), 6.0f,
                       ImDrawFlags_RoundCornersBottom, 1.0f);
  }

  bool dragging = false;
  if (const bool dragging =
          ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    is_dragging_ = true;
    ImGui::Image(ui::GetTextureID(icon_), ImVec2(20, 20));
    ImGui::SameLine();
    const auto& name = display_name_;
    ImGui::TextUnformatted(name.c_str());
    result.Set(AssetsBrowserAction::kSelect, true);

    ImGui::SetDragDropPayload("asset_payload", &handle_, sizeof(AssetHandle));

    ImGui::EndDragDropSource();
  }
  if (ImGui::IsItemHovered()) {
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      result.Set(AssetsBrowserAction::kActivated, true);
    } else {
      const bool action = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
      is_selected = SelectManager::Instance()->IsSelectItem("", handle_);
      const bool skip_because_dragging = is_dragging_ && is_selected;
      if (action && !skip_because_dragging) {
        if (just_selected_) just_selected_ = false;

        if (!is_selected) {
          result.Set(AssetsBrowserAction::kSelect, true);
          just_selected_ = true;
        }
      }
    }
  }
  is_dragging_ = dragging;

  return result;
}

void AssetsBrowserItem::OnRenderEnd() { ImGui::PopID(); }

AssetsBrowserDirectory::AssetsBrowserDirectory(
    const Ref<DirectoryInfo>& directory_info)
    : AssetsBrowserItem(ItemType::kDirectory, directory_info->handle,
                        directory_info->filepath.filename().string(),
                        ThemeDB::GetInstance()->GetIcon("DefaultFolder")),
      directory_info_(directory_info) {}

AssetsBrowserAsset::AssetsBrowserAsset(const AssetMetadata& asset_info,
                                       const Ref<Texture>& icon)
    : AssetsBrowserItem(ItemType::kAsset, asset_info.handle,
                        asset_info.file_path.filename().string(), icon),
      asset_info_(asset_info) {}

}  // namespace base_engine::editor
