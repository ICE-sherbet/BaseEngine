#include "TagComponent.h"

namespace base_engine::component {
void TagComponent::_Bind() {
  ComponentDB::BindMethod("SetTag", &TagComponent::SetTag);
  ComponentDB::BindMethod("GetTag", &TagComponent::Tag);

  ADD_PROPERTY(PropertyInfo(VariantType::kString, "tag"), "SetTag", "GetTag");
}
}  // namespace base_engine::component
