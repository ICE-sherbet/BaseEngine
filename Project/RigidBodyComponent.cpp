#include "RigidBodyComponent.h"

#include "VelocityComponent.h"

namespace base_engine::physics {
void RigidBodyComponent::_Bind() { REQUIRE_COMPONENT(VelocityComponent); }
}  // namespace base_engine::physics
