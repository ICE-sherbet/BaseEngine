// @EditorProperties.h
// @brief
// @author ICE
// @date 2023/06/27
//
// @details

#pragma once
#include "EditorAssetPicker.h"
#include "EditorCheckBox.h"
#include "EditorDragValue.h"
#include "EditorProperty.h"
namespace base_engine::editor {
class EditorPropertyCheck : public EditorProperty {
 public:
  EditorPropertyCheck();
  void UpdateProperty() override;

 private:
  void CheckBoxPressed();

 private:
  std::shared_ptr<EditorCheckBox> checkbox_;
};

class EditorPropertyVector2 : public EditorProperty {
 public:
  EditorPropertyVector2();

  void Draw() const override;

  void UpdateProperty() override;

 private:
  void ValueChanged();

 private:
  std::array<std::shared_ptr<EditorDragValue>, 2> drag_values_;
};

class EditorPropertyVector3 : public EditorProperty {
 public:
  EditorPropertyVector3();

  void Draw() const override;
  void UpdateProperty() override;

 private:
  void ValueChanged(float dummy);

 private:
  std::array<std::shared_ptr<EditorDragValue>, 3> drag_values_;
};

class EditorPropertyAsset : public EditorProperty {
 public:
  EditorPropertyAsset();

  void Draw() const override;
  void Initialize(const std::string& type) const;
  void UpdateProperty() override;

 private:
  void AssetSelected(AssetHandle handle);

private:
  std::shared_ptr<EditorAssetPicker> asset_picker_;
};

}  // namespace base_engine::editor
