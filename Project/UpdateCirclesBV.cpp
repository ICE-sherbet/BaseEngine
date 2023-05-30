#include "UpdateCirclesBV.h"
namespace base_engine::physics {
void UpdateCirclesBV::Process(becs::Entity entity, const Vector3& pos,
                              const Circle& shape, BoundingBox& box) {
  box = BoundingBox::ConstructBoundingBox({pos.x, pos.y}, shape.radius);
}
}  // namespace base_engine::physics