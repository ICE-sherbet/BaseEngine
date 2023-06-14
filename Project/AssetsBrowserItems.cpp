#include "AssetsBrowserItems.h"

#include "AssetsBrowserSetting.h"
#include "EditorTextureResource.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace base_engine::editor {
ContentBrowserItem::ContentBrowserItem(const ItemType type,
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

void ContentBrowserItem::OnRenderBegin() {
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
ContentBrowserItemActionResult ContentBrowserItem::OnRender() {
  ItemActionResult result;
  const float thumbnail_size = AssetsBrowserSetting::Get().thumbnail_size;
  ImGui::InvisibleButton("##thumbnailButton",
                         ImVec2{thumbnail_size, thumbnail_size});
  const auto size = RectExpanded(
      ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()), -6.0f, -6.0f);
  ImGui::GetWindowDrawList()->AddImage(
      icon_->GetTexture(), size.Min, size.Max, ImVec2(0, 0), ImVec2(1, 1),
      ImGui::IsItemHovered() ? ImColor(255, 255, 255, 255)
                             : ImColor(255, 255, 255, 225));
  ImGui::Text(display_name_.c_str());

  ImGui::EndGroup();

  bool isSelected = false;
  if (isSelected || ImGui::IsItemHovered()) {
    auto itemRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    auto* drawList = ImGui::GetWindowDrawList();

    if (isSelected) {
      const bool mouseDown =
          ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsItemHovered();
      ImColor colTransition = IM_COL32(237, 192, 119, 255);

      drawList->AddRect(
          itemRect.Min, itemRect.Max,
          mouseDown ? ImColor(colTransition)
                    : ImColor(IM_COL32(237, 192, 119, 255)),
          6.0f,
          type_ == ItemType::kDirectory ? 0 : ImDrawFlags_RoundCornersBottom,
          1.0f);
    } else  // isHovered

    // Outline
    {
      drawList->AddRect(itemRect.Min, itemRect.Max,
                        ImColor(IM_COL32(237, 192, 119, 255)), 6.0f,
                        ImDrawFlags_RoundCornersBottom, 1.0f);
    }
  }

  return result;
}

void ContentBrowserItem::OnRenderEnd() { ImGui::PopID(); }

ContentBrowserDirectory::ContentBrowserDirectory(
    const Ref<DirectoryInfo>& directory_info)
    : ContentBrowserItem(ItemType::kDirectory, directory_info->handle,
                         directory_info->filepath.filename().string(),
                         ThemeDB::GetInstance()->GetIcon("DefaultFolder")),
      directory_info_(directory_info) {}

ContentBrowserAsset::ContentBrowserAsset(const AssetMetadata& asset_info,
                                         const Ref<Texture>& icon)
    : ContentBrowserItem(ItemType::kAsset, asset_info.handle,
                         asset_info.file_path.filename().string(), icon),
      asset_info_(asset_info) {}

ContentBrowserAsset::~ContentBrowserAsset() {}
}  // namespace base_engine::editor
