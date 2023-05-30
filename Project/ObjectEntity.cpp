#include "ObjectEntity.h"

void base_engine::ObjectEntity::SetParent(const ObjectEntity parent) {
  SetParent(parent, true);
}

void base_engine::ObjectEntity::SetParent(ObjectEntity parent,
                                          const bool world_position_stays) {
  ObjectEntity current_parent = this->GetParent();
  if (current_parent == parent) return;

  if (current_parent) current_parent.RemoveChild(*this);

  SetParentUUID(parent.GetUUID());

  if (!parent) return;
  auto& parent_children = parent.Children();
  if (const UUID uuid = GetUUID();
      std::ranges::find(parent_children, uuid) != parent_children.end())
    return;
  parent_children.emplace_back(GetUUID());
  parent.GetComponent<component::TransformComponent>().SetChildren(
      parent_children);


  if (!world_position_stays) return;
  const auto world_transform_matrix =
      scene_->GetWorldSpaceTransformMatrix(parent);
  auto& transform = GetComponent<component::TransformComponent>();
  const auto local = transform.GetLocalTransform();
  const auto matrix = mat::Inverse(world_transform_matrix) * transform.GetGlobalTransform();

  transform.SetGlobalTransform(matrix);
}

void base_engine::ObjectEntity::SetParentUUID(const UUID& parent) {
  GetComponent<component::HierarchyComponent>().parent_handle = parent;
  GetComponent<component::TransformComponent>().SetParent(parent);
}
