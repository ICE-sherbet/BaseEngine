// @DynamicContactSolver.h
// @brief
// @author ICE
// @date 2023/05/10
//
// @details

#pragma once
#include "ContactSolverSystem.h"
#include "RigidBodyComponent.h"
#include "VelocityComponent.h"

namespace base_engine::physics {
template <typename TagA, typename TagB>
class DynamicContactSolver : public ContactSolverSystem<TagA, TagB> {
 public:
  DynamicContactSolver(const Ref<Scene>& scene,
                       const Ref<PhysicsEngineData>& engine)
      : ContactSolverSystem<TagA, TagB>(scene, engine) {}

 protected:
  void SolveContact(SolveContact<TagA, TagB>& contact) override {
    auto view = this->GetScene()
                    ->GetRegistry()
                    .template view<RigidBodyComponent, VelocityComponent,
                                   component::TransformComponent>();
    /*
    auto& [velocity_a, transform_a] =
        view.template get<VelocityComponent, component::TransformComponent>(
            contact.body_a);
    auto& [velocity_b, transform_b] =
        view.template get<VelocityComponent, component::TransformComponent>(
            contact.body_b);

    const auto& rigid_a = view.template get<RigidBodyComponent>(contact.body_a);
    const auto& rigid_b = view.template get<RigidBodyComponent>(contact.body_b);
    const auto restitution =
        (std::min)(rigid_a.restitution, rigid_b.restitution);
        */
  }
};
}  // namespace base_engine::physics
