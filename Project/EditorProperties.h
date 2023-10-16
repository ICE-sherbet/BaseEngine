// @EditorProperties.h
// @brief
// @author ICE
// @date 2023/06/27
//
// @details

#pragma once
#include "EditorAssetPicker.h"
#include "EditorCheckBox.h"
#include "EditorDragValueFloat.h"
#include "EditorDragValueInt.h"
#include "EditorProperty.h"
#include "EditorTextBox.h"

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
class EditorPropertyInteger : public EditorProperty {
 public:
  EditorPropertyInteger();
  void Setup(int64_t min, int64_t max, int64_t step);

  void Draw() const override;

  void UpdateProperty() override;

 private:
  void ValueChanged();

 private:
  std::shared_ptr<EditorDragValueInt> drag_value_;
};
class EditorPropertyFloat : public EditorProperty {
 public:
  EditorPropertyFloat();

  void Draw() const override;

  void UpdateProperty() override;

 private:
  void ValueChanged();

 private:
  std::shared_ptr<EditorDragValueFloat> drag_value_;
};
class EditorPropertyVector2 : public EditorProperty {
 public:
  EditorPropertyVector2();

  void Draw() const override;

  void UpdateProperty() override;

 private:
  void ValueChanged();

 private:
  std::array<std::shared_ptr<EditorDragValueFloat>, 2> drag_values_;
};

class EditorPropertyVector3 : public EditorProperty {
 public:
  EditorPropertyVector3();

  void Draw() const override;
  void UpdateProperty() override;

 private:
  void ValueChanged(float dummy);

 private:
  std::array<std::shared_ptr<EditorDragValueFloat>, 3> drag_values_;
};

class EditorPropertyText : public EditorProperty {
 public:
  EditorPropertyText();

  void Draw() const override;
  void UpdateProperty() override;

 private:
  void TextChanged(const std::string& text);

 private:
  std::shared_ptr<EditorTextBox> text_box_;
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

class EditorPropertyLayers : public EditorProperty {
  class EditorPropertyLayersCombo;
 public:
  EditorPropertyLayers();
  ~EditorPropertyLayers();

  void Draw() const override;
  void UpdateProperty() override;

 private:
  void ValueChanged();

 private:
  std::shared_ptr<EditorPropertyLayersCombo> combo_box_;
};
}  // namespace base_engine::editor
