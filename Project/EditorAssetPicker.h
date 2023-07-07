// @EditorAssetPicker.h
// @brief
// @author ICE
// @date 2023/06/29
//
// @details

#pragma once
#include "EditorButton.h"
#include "EditorControl.h"
#include "EditorLabel.h"

namespace base_engine::editor {
class EditorAssetPicker : public EditorControl {
 public:
  EditorAssetPicker(const std::string& text = "", const std::string& type = "");

  void Notification(int type) override;

  /**
   * \brief 検索対称のタイプを指定する
   * \param type 検索対称のタイプ 空で全検索
   */
  void SetSearchType(const std::string& type) { search_type_ = type; }

  void SetEditedAsset(AssetHandle handle);

private:
  void UpdateMenu();
  void Popup();
  void Draw();
  void PopupDraw();
  void AssetSelect();
  void EditedAssetUpdate() const;
  void Cancel();
  void ClosePopup();
  std::string text_;
  std::string search_type_;

  AssetHandle pick_ = 0;

  std::shared_ptr<EditorLabel> label_;
  std::shared_ptr<EditorButton> edit_button_;
  std::shared_ptr<EditorButton> assign_button_;
  std::shared_ptr<EditorButton> cancel_button_;

  bool show_popup_ = false;
  const char* draw_id_;
};
}  // namespace base_engine::editor
