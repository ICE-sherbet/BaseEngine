// @ImGuiUtilities.h
// @brief
// @author ICE
// @date 2023/06/20
//
// @details

#pragma once
#include <algorithm>

#include "imgui.h"
#include "imgui_internal.h"

namespace base_engine::editor::ui {

class ScopedColorStack {
 public:
  ScopedColorStack(const ScopedColorStack&) = delete;
  ScopedColorStack& operator=(const ScopedColorStack&) = delete;

  template <typename ColorType, typename... OtherColors>
  ScopedColorStack(ImGuiCol first_color_id, ColorType first_color,
                   OtherColors&&... other_color_pairs)
      : count_((sizeof...(other_color_pairs) / 2) + 1) {
    PushColor(first_color_id, first_color,
              std::forward<OtherColors>(other_color_pairs)...);
  }

  ~ScopedColorStack() { ImGui::PopStyleColor(count_); }

 private:
  int count_;

  template <typename ColorType, typename... OtherColors>
  void PushColor(ImGuiCol color_id, ColorType color,
                 OtherColors&&... other_color_pairs) {
    if constexpr (sizeof...(other_color_pairs) == 0) {
      ImGui::PushStyleColor(color_id, ImColor(color).Value);
    } else {
      ImGui::PushStyleColor(color_id, ImColor(color).Value);
      PushColor(std::forward<OtherColors>(other_color_pairs)...);
    }
  }
};

inline ImColor ColorWithMultipliedValue(const ImColor& color,
                                        const float multiplier) {
  const ImVec4& col_raw = color.Value;
  float hue, sat, val;
  ImGui::ColorConvertRGBtoHSV(col_raw.x, col_raw.y, col_raw.z, hue, sat, val);
  return ImColor::HSV(hue, sat, std::min(val * multiplier, 1.0f));
}

inline ImRect RectExpanded(const ImRect& rect,float expand_x,float expand_y)
{
  ImRect result = rect;
  result.Expand({expand_x, expand_y});
  return result;
}

inline ImRect GetItemRect()
{
  return {ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
}

/**
 * \brief 画像ボタンを表示させる。
 * \param texture テクスチャ
 * \param tint_normal デフォルト時の色合い
 * \param tint_hovered ホバー時の色合い
 * \param tint_pressed 押下時の色合い
 * \param min_rect 描画範囲 左上
 * \param max_rect 描画範囲 右下
 */
inline void DrawButtonImage(ImTextureID texture, ImU32 tint_normal,
                            ImU32 tint_hovered, ImU32 tint_pressed,
                            const ImVec2& min_rect, const ImVec2& max_rect) {
  auto* drawList = ImGui::GetWindowDrawList();
  if (ImGui::IsItemActive())
    drawList->AddImage(texture, min_rect, max_rect, ImVec2(0, 0), ImVec2(1, 1),
                       tint_pressed);
  else if (ImGui::IsItemHovered())
    drawList->AddImage(texture, min_rect, max_rect, ImVec2(0, 0), ImVec2(1, 1),
                       tint_hovered);
  else
    drawList->AddImage(texture, min_rect, max_rect, ImVec2(0, 0), ImVec2(1, 1),
                       tint_normal);
}

/**
 * \brief 画像ボタンを表示させる。
 * \param texture テクスチャ
 * \param tint_normal デフォルト時の色合い
 * \param tint_hovered ホバー時の色合い
 * \param tint_pressed 押下時の色合い
 * \param rectangle 描画範囲
 */
inline void DrawButtonImage(ImTextureID texture, ImU32 tint_normal,
                            ImU32 tint_hovered, ImU32 tint_pressed,
                            const ImRect& rectangle) {
  DrawButtonImage(texture, tint_normal, tint_hovered, tint_pressed,
                  rectangle.Min, rectangle.Max);
}

}  // namespace base_engine::editor::ui