#include "EditorProperty.h"
namespace base_engine::editor {
std::string EditorProperty::GetPropertyName() { return property_name_; }

void EditorProperty::EmitChanged(const std::string& class_name,
                                 const std::string& property,
                                 const Variant& value) {
  EmitSignal("PropertyChanged", class_name,property,value);
}

void EditorProperty::UpdateProperty() {}
}  // namespace base_engine::editor
