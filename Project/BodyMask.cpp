#include "BodyMask.h"

namespace base_engine::physics {
void BodyMask::_Bind() {
  ComponentDB::BindMethod("SetBodyMask", &BodyMask::SetBodyMask);
  ComponentDB::BindMethod("GetBodyMask", &BodyMask::GetBodyMask);
  ADD_PROPERTY(PropertyInfo(VariantType::kInt32, "body_mask",
                            PropertyHint::kLayerPhysics),
               "SetBodyMask", "GetBodyMask");

  ComponentDB::BindMethod("SetTargetMask", &BodyMask::SetTargetMask);
  ComponentDB::BindMethod("GetTargetMask", &BodyMask::TargetMask);
  ADD_PROPERTY(PropertyInfo(VariantType::kInt32, "target_mask",
                            PropertyHint::kLayerPhysics),
               "SetTargetMask", "GetTargetMask");


}
}  // namespace base_engine::physics
