// @ContactSolverReferenceService.h
// @brief
// @author ICE
// @date 2023/05/10
//
// @details

#pragma once
#include "ObjectEntity.h"
#include "Scene.h"
#include "SolveContact.h"
#include "Vector2.h"

namespace base_engine::physics {
class ContactSolverReferenceService {
 public:
  virtual void AddTask(becs::Entity body_a, becs::Entity body_b, int tag_a,
                       int tag_b, float penetration, const Vector2& normal,
                       const Vector2& point) = 0;
};

template <typename TagA, typename TagB>
class ContactSolverReference : public ContactSolverReferenceService {
public:
  explicit ContactSolverReference(const Ref<Scene> scene) {
    expected_first_tag_type_id_ = TagA::Type();
    scene_ = scene;
  }

 public:
  void AddTask(const becs::Entity body_a, const becs::Entity body_b,
               const int tag_a, [[maybe_unused]] int tag_b,
               const float penetration, const Vector2& normal,
               const Vector2& point) override {
    if (tag_a == expected_first_tag_type_id_) {
      AddTaskImpl(body_a, body_b, penetration, normal, point);
    } else {
      AddTaskImpl(body_b, body_a, penetration, -normal, point);
    }
  }

 private:
  void AddTaskImpl(const becs::Entity body_a, const becs::Entity body_b,
                   float penetration, Vector2 normal, Vector2 point) {
    auto& registry = scene_->GetRegistry();
    const auto entity = registry.create();
    registry.emplace<SolveContact<TagA, TagB>>(entity, body_a, body_b,
                                               penetration, normal, point);
  }

  Ref<Scene> scene_;
  int expected_first_tag_type_id_;
};
}  // namespace base_engine::physics
