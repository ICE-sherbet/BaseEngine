// @ContactTesterSystem.h
// @brief
// @author ICE
// @date 2023/05/09
//
// @details

#pragma once
#include "AbstractPhysicsSystem.h"
#include "ContactSolverReferenceService.h"
#include "OnCollisionTag.h"

namespace base_engine::physics {
template <typename ShapeA, typename ShapeB>
class ContactTesterSystem
    : public AbstractPhysicsSystem<ContactTesterSystem<ShapeA, ShapeB>> {
 protected:
  using Contact = PossibleContact<ShapeA, ShapeB>;

 public:
  ContactTesterSystem(const Ref<Scene>& scene,
                      const Ref<PhysicsEngineData>& engine)
      : AbstractPhysicsSystem<ContactTesterSystem<ShapeA, ShapeB>>(*this, scene,
                                                                   engine) {}

  void OnInit() override {
    tester_table_ = &this->GetEngineData()->physics_data_.contact_tester_table;
    solver_table_ = &this->GetEngineData()->physics_data_.contact_solver_table;
    RegisterTester();
  }
  void OnUpdate() override {
    auto& registry = this->GetScene()->GetRegistry();
    for (auto view = registry.template view<Contact>();
         const auto entity : view) {
      auto& contact = view.template get<Contact>(entity);
      TestPair(contact);
      registry.destroy(entity);
    }
  }

 protected:
  virtual void TestPair(Contact& possible_contact) = 0;
  void AddSolverTask(Contact& possible_contact, float penetration,
                     Vector2 normal, Vector2 point) {
    const auto tag_a = possible_contact.tag_a;
    const auto tag_b = possible_contact.tag_b;
    auto& registry = this->GetScene()->GetRegistry();
    registry.template emplace<OnCollisionEnterTag>(possible_contact.body_a);
    registry.template emplace<OnCollisionEnterTag>(possible_contact.body_b);
    (*solver_table_)[tag_a][tag_b]->AddTask(possible_contact.body_a,
                                            possible_contact.body_b, tag_a,
                                            tag_b, penetration, normal, point);
  }

 private:
  void RegisterTester() {
    const auto a = ShapeA::Type();
    const auto b = ShapeB::Type();

    (*tester_table_)[a][b] =
        std::make_unique<ContactTesterReference<ShapeA, ShapeB>>(
            this->GetScene());
    (*tester_table_)[b][a] =
        std::make_unique<ContactTesterReference<ShapeA, ShapeB>>(
            this->GetScene());
  }
  ContactTesterReferenceServiceTable* tester_table_;
  ContactSolverReferenceServiceTable* solver_table_;
};
}  // namespace base_engine::physics
