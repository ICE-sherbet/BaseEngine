// @ContactTesterReferenceService.h
// @brief
// @author ICE
// @date 2023/05/09
//
// @details

#pragma once
#include "BodyMask.h"
#include "ObjectEntity.h"
#include "PossibleContact.h"
#include "Scene.h"
#include "ShapeComponents.h"

namespace base_engine::physics {
class ContactTesterReferenceService {
 public:
  explicit ContactTesterReferenceService(const Ref<Scene>& scene_context)
      : scene_context_(scene_context) {}

  virtual void AddTask(becs::Entity body_a, becs::Entity body_b,
                       BodyMask mask_a, BodyMask mask_b) = 0;

 protected:
  Ref<Scene> scene_context_;
};
template <typename ShapeA, typename ShapeB>
class ContactTesterReference : public ContactTesterReferenceService {
 public:
  explicit ContactTesterReference(const Ref<Scene>& scene_context)
      : ContactTesterReferenceService(scene_context) {}

  void AddTask(becs::Entity body_a, becs::Entity body_b, BodyMask mask_a,
               BodyMask mask_b) override {
    if (mask_a.shape_type_id == ShapeA::Type()) {
      AddTaskImpl(body_a, body_b, mask_a.tag_type_id, mask_b.tag_type_id);
    } else {
      AddTaskImpl(body_b, body_a, mask_b.tag_type_id, mask_a.tag_type_id);
    }
  }

 private:
  void AddTaskImpl(becs::Entity body_a, becs::Entity body_b, int mask_a,
                   int mask_b) {
    const auto entity = scene_context_->GetRegistry().create();
    scene_context_->GetRegistry().template emplace<PossibleContact<ShapeA, ShapeB>>(
        entity, body_a, body_b, mask_a, mask_b);
  }
};
}  // namespace base_engine::physics
