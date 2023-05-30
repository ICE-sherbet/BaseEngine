#include "IntegratePosesSystem.h"

#include "RigidBodyComponent.h"
#include "VelocityComponent.h"
#define max(a, b)
namespace base_engine::physics {
IntegratePosesSystem::IntegratePosesSystem(const Ref<Scene>& scene,
                                           const Ref<PhysicsEngineData> engine)
    : AbstractPhysicsSystem<IntegratePosesSystem>(*this, scene, engine) {}

void IntegratePosesSystem::OnInit() {}

void IntegratePosesSystem::OnUpdate() {
  // TODO SharedDataの概念を実装する
  /*
  for (const auto bodies =
           GetScene()
               ->GetAllEntitiesWith<component::TransformComponent,
                                    RigidBodyComponent, VelocityComponent>();
       const auto& body : bodies) {
    constexpr float dt = 0.017f;
    auto& transform = bodies.get<component::TransformComponent>(body);

    if (transform.IsDirty()) {
      GetScene()->GetWorldSpaceTransformMatrix(
          {body, GetScene().Raw()});
    }

    const auto position = transform.GetLocalTranslation();
    auto& rigid_body = bodies.get<RigidBodyComponent>(body);
    auto& velocity = bodies.get<VelocityComponent>(body);
    const auto linearAcceleration = velocity.force * rigid_body.inverse_mass;
    const auto angularAcceleration = velocity.torque * rigid_body.inverse_MMOI;
    {
      const auto pos =
          velocity.linear * dt + linearAcceleration * dt * dt * 0.5f;
      transform.SetLocalTranslation(position + Vector3{pos.x, pos.y, 0});
    }

    const auto angle = transform.GetLocalRotationEuler();
    transform.SetLocalRotationEuler({angle.x, angle.y,
                                angle.z + velocity.angular * dt +
                                    angularAcceleration * dt * dt * 0.5f});

    velocity.linear += linearAcceleration * dt;
    velocity.angular += angularAcceleration * dt;

    transform.rotation_vector_ = Vector2{cos(angle.z), sin(angle.z)};

    velocity.force = Vector2(0, 0);
    velocity.torque = 0;
  }
  */
}
}  // namespace base_engine::physics
