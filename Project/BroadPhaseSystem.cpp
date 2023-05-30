#include "BroadPhaseSystem.h"

#include "BoundingBox.h"
#include "Profiler.h"

namespace base_engine::physics {
void BroadPhaseSystem::OnInit() {}

void BroadPhaseSystem::OnUpdate() {
  BE_PROFILE_FUNC("BroadPhase");
  SortBoundingBoxes();
  auto& reg = GetScene()->GetRegistry();
  const auto view = reg.view<BoundingBox, BodyMask>();
  const auto& handle = view.handle();
  const auto bodies_count = handle.size();

  const auto& testersMatrix =
      GetEngineData()->physics_data_.contact_tester_table;
  const auto& solversMatrix =
      GetEngineData()->physics_data_.contact_solver_table;

  for (int i = 0; i < bodies_count; ++i) {
    const auto entity_a = handle[i];
    const auto body_a = view.get<BoundingBox>(entity_a);

    for (int k = i + 1; k < bodies_count; ++k) {
      const auto entity_b = handle[k];
      const auto body_b = view.get<BoundingBox>(entity_b);
      const auto mask_a = view.get<BodyMask>(entity_a);
      const auto mask_b = view.get<BodyMask>(entity_b);
      if (body_b.min.x > body_b.max.x) {
        break;
      }
      if (!(mask_a.body_mask & mask_b.target_mask)) {
        break;
      }
      if (!(mask_a.target_mask & mask_b.body_mask)) {
        break;
      }

      if (body_a.max.y >= body_b.min.y && body_b.max.y >= body_a.min.y) {
        const auto& tester_reference =
            testersMatrix[mask_a.shape_type_id][mask_b.shape_type_id];
        const auto& solver_reference =
            solversMatrix[mask_a.tag_type_id][mask_b.tag_type_id];
        if (tester_reference != nullptr && solver_reference != nullptr) {
          tester_reference->AddTask(entity_a, entity_b, mask_a, mask_b);
        }
      }
    }
  }
}

struct Compare {
  bool operator()(const BoundingBox& a, const BoundingBox& b) const {
    return a.min.x < b.min.x;
  }
};
void BroadPhaseSystem::SortBoundingBoxes() {
  GetScene()->GetRegistry().sort<BoundingBox, Compare>(Compare{});
}
}  // namespace base_engine::physics
