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
#include "ObjectEntity.h"
#include "Scene.h"

namespace base_engine::editor {
class EditorProperty : public ConnectableObject {
 protected:
  enum {
    kControlDraw = 100,
  };

 public:
  void SetPropertyName(const std::string& name);

  void SetObjectAndClassName(ObjectEntity& object,const std::string& class_name);
  std::string GetPropertyName();

  void EmitChanged(const std::string& class_name,const std::string& property,
                   const Variant& value);
  virtual void UpdateProperty();

  virtual void Draw() const;

  EditorProperty();;

  void AddControl(std::shared_ptr<EditorControl> control);

protected:
  ObjectEntity object_;
  std::string class_name_;
  std::string property_name_;
  std::shared_ptr<EditorLabel> label_;
  std::vector<std::shared_ptr<EditorControl>> controls_;
};
}  // namespace base_engine::editor
