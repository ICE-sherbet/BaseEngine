#include "ShapeComponents.h"

#include "ComponentProperty.h"
#include "ScriptTypes.h"

namespace base_engine::physics {
void CircleShape::_Bind() {
  ComponentDB::BindMethod("SetRadius", &CircleShape::SetRadius);
  ComponentDB::BindMethod("GetRadius", &CircleShape::Radius);
  ADD_PROPERTY(PropertyInfo(VariantType::kFloat, "radius"), "SetRadius",
               "GetRadius");
}
}  // namespace base_engine::physics
