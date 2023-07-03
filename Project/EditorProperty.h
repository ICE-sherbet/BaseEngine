// @EditorProperty.h
// @brief
// @author ICE
// @date 2023/06/27
//
// @details

#pragma once
#include "ConnectableObject.h"
#include "EditorControl.h"
#include "EditorLabel.h"

namespace base_engine::editor {
class EditorProperty : public ConnectableObject {
 protected:
  enum {
    kControlDraw = 100,
  };

 public:
  void SetPropertyName(const std::string& name) {
    property_name_ = name;
    label_->SetText(property_name_);
  }
  void SetObjectAndClassName(void* object,const std::string& class_name)
  {
    object_ = object;
    class_name_ = class_name;
  }
  std::string GetPropertyName();

  void EmitChanged(const std::string& class_name,const std::string& property,
                   const Variant& value);
  virtual void UpdateProperty();

  virtual void Draw() const {
    label_->Notification(kControlDraw);
    for (const auto& control : controls_) {
      control->Notification(kControlDraw);
    }
  }

  EditorProperty() { label_ = std::make_shared<EditorLabel>(); };

  void AddControl(std::shared_ptr<EditorControl> control) {
    controls_.emplace_back(control);
  }

 protected:
  void* object_ = nullptr;
  std::string class_name_;
  std::string property_name_;
  std::shared_ptr<EditorLabel> label_;
  std::vector<std::shared_ptr<EditorControl>> controls_;
};
}  // namespace base_engine::editor
