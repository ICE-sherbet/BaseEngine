#include "ContactTesterCircleCircle.h"

base_engine::physics::ContactTesterCircleCircle::ContactTesterCircleCircle(const Ref<Scene>& scene,
	const Ref<PhysicsEngineData>& engine): ContactTesterSystem<CircleShape, CircleShape>(scene, engine)
{}

void base_engine::physics::ContactTesterCircleCircle::TestPair(Contact& possible_contact)
{
	const auto transform_view =
		GetScene()->GetRegistry().view<component::TransformComponent>();
	const auto circle_view = GetScene()->GetRegistry().view<physics::CircleShape>();

	auto pos_a =
		transform_view
		.get<component::TransformComponent>(possible_contact.body_a)
		.GetGlobalTranslation();
	auto pos_b =
		transform_view
		.get<component::TransformComponent>(possible_contact.body_b)
		.GetGlobalTranslation();

	const auto [radius_a] =
		circle_view.get<physics::CircleShape>(possible_contact.body_a);
	const auto [radius_b] =
		circle_view.get<physics::CircleShape>(possible_contact.body_b);

	float penetration = 0;
	Vector2 normal;
	Vector2 point;

	if (PhysicsTesterCommon::CircleCircle(radius_a, {pos_a.x, pos_a.y},
	                                      radius_b, {pos_b.x, pos_b.y},
	                                      penetration, normal, point)) {
		AddSolverTask(possible_contact, penetration, normal, point);
	}
}
