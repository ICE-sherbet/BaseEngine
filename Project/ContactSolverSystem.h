// @ContactSolverSystem.h
// @brief
// @author ICE
// @date 2023/05/10
//
// @details

#pragma once
#include "AbstractPhysicsSystem.h"
#include "SolveContact.h"
namespace base_engine::physics {
template <typename TagA, typename TagB>
class ContactSolverSystem
    : public AbstractPhysicsSystem<ContactSolverSystem<TagA, TagB>> {
  using Contact = SolveContact<TagA, TagB>;
 public:
  ContactSolverSystem(const Ref<Scene>& scene,
                      const Ref<PhysicsEngineData>& engine)
      : AbstractPhysicsSystem<ContactSolverSystem<TagA, TagB>>(*this, scene,
                                                               engine) {}

  void OnInit() override {
    RegisterSolver(this->GetEngineData()->physics_data_.contact_solver_table);
  }
  void OnUpdate() override {
    auto& registry = this->GetScene()->GetRegistry();
    for (auto view = registry.template view<Contact>(); const auto entity : view) {
      auto& contact = view.template get<Contact>(entity);
      SolveContact(contact);
      registry.destroy(entity);
    }
  }

 protected:
  virtual void SolveContact(SolveContact<TagA, TagB>& contact) = 0;

 private:
  void RegisterSolver(ContactSolverReferenceServiceTable& solversMatrix) {
    const auto a = TagA::Type();
    const auto b = TagB::Type();
    solversMatrix[a][b] =
        std::make_unique<ContactSolverReference<TagA, TagB>>(this->GetScene());
    solversMatrix[b][a] =
        std::make_unique<ContactSolverReference<TagA, TagB>>(this->GetScene());
  }
};
}  // namespace base_engine::physics
