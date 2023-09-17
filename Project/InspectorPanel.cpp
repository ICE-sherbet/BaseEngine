#include "InspectorPanel.h"

#include <imgui.h>

#include <numbers>

#include "BodyMask.h"
#include "CSharpScriptEngine.h"
#include "ImGuiAssetHelper.h"
#include "ImGuiUtilities.h"
#include "ImGuiVariantHelper.h"
#include "InspectorUtilities.h"
#include "MonoScriptUtilities.h"
#include "PhysicsObjectFactory.h"
#include "RigidBodyComponent.h"
#include "SelectManager.h"
#include "ShapeComponents.h"
namespace base_engine::editor {

void InspectorPanel::Initialize(const Ref<Scene>& context) {
  SetSceneContext(context);
  SelectManager::Instance()->Connect(
      "SelectItem",
      make_callable_function_pointer(this, &InspectorPanel::PickItem));

  fixed_class_.emplace_back(component::TagComponent::_GetClassNameStatic());
  fixed_class_.emplace_back(
      component::TransformComponent::_GetClassNameStatic());

  context_menu_ = std::make_unique<InspectorContextMenu>("Component Menu");
  context_menu_->BuildRoot();
  context_menu_->Connect("OnRemovedComponent",
                         make_callable_function_pointer(
                             this, &InspectorPanel::OnRemovedComponent));
}

void InspectorPanel::OnImGuiRender() {
  {
    ImGui::Begin("Inspector");
    if (select_item_ != 0) {
      RenderProperties();
    }

    ImGui::End();
  }
}

void InspectorPanel::RenderProperties() {
  ObjectEntity object = scene_context_->TryGetEntityWithUUID(select_item_);
  if (!object) return;
  if (!object.HasComponent<component::TagComponent>()) {
    return;
  }
  if (redraw_) GenerateProperties(object);
  ui::PushID();
  ImGui::BeginGroup();

  const auto ComponentDraw = [this](const std::string& clazz) {
    const auto open =
        ImGui::CollapsingHeader(clazz.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
    // TODO RemoveComponentを実装する
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      context_menu_->SetTargetObject(scene_context_, select_item_,
                                     ComponentDB::GetClass(clazz)->id);
      context_menu_->Show();
    }

    if (!open) return;
    for (const auto& property : properties_map_[clazz]) {
      property->UpdateProperty();
      property->Draw();
    }
  };

  std::map<std::string, bool> drawing_class;
  for (const auto& fixed_class : fixed_class_) {
    drawing_class[fixed_class] = true;
    ComponentDraw(fixed_class);
  }
  for (const auto& class_name : properties_map_ | std::views::keys) {
    if (drawing_class.contains(class_name)) continue;
    ComponentDraw(class_name);
  }

  context_menu_->RenderBegin();
  context_menu_->Render();
  context_menu_->RenderEnd();
  ImGui::EndGroup();
  RenderAddComponentButton(object);

  ui::PopID();
}
template <typename Component>
bool AddComponentButton(const char* label, ObjectEntity& object) {
  if (object.HasComponent<Component>()) return false;
  if (!ImGui::MenuItem(label)) return false;
  object.AddComponent<Component>();
  ImGui::CloseCurrentPopup();
  return true;
}
template <typename... Components>
bool AddComponentsButton(const char* label, ObjectEntity& object) {
  if (object.HasComponent<Components...>()) return false;
  if (!ImGui::MenuItem(label)) return false;
  object.AddComponent<Components...>();
  ImGui::CloseCurrentPopup();
  return true;
}
template <typename... Components, typename OnAddedFunc>
bool AddComponentsButtonEvent(const char* label, ObjectEntity& object,
                              OnAddedFunc on_added_func) {
  if (object.HasComponent<Components...>()) return false;
  if (!ImGui::MenuItem(label)) return false;
  auto components = std::make_tuple(object.AddComponent<Components...>());
  ImGui::CloseCurrentPopup();
  on_added_func(object, components);
  return true;
}
void InspectorPanel::RenderAddComponentButton(ObjectEntity& object) {
  ImGui::Separator();

  if (ImGui::Button("Add Component")) {
    ImGui::OpenPopup("AddComponent");
  }

  if (ImGui::BeginPopup("AddComponent")) {
    using namespace component;
    auto& r = scene_context_->GetRegistry();
    std::list<uint32_t> class_list;
    ComponentDB::GetClassList(&class_list);
    for (const uint32_t hash : class_list) {
      if (r.valid(hash) && r.storage(hash)->contains(object.GetHandle()))
        continue;
      const auto clazz = ComponentDB::GetClass(hash);
      if (!ImGui::MenuItem(clazz->name.c_str())) continue;
      if (!r.valid(hash)) {
        const auto require_component = ComponentDB::GetClass(hash);
        r.create_pool(hash, require_component->registry_pool_factory);
      }
      r.storage(hash)->try_emplace(object.GetHandle(), false);
      std::list<uint32_t> require_list;
      ComponentDB::GetRequireComponent(clazz->id, &require_list);
      for (const auto require : require_list) {
        if (!r.valid(require)) {
          const auto require_component = ComponentDB::GetClass(require);
          r.create_pool(require, require_component->registry_pool_factory);
        }
        if (r.storage(require)->contains(object.GetHandle())) continue;
        r.storage(require)->try_emplace(object.GetHandle(), false);
      }

      GenerateProperties(object);

      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void FilterCombo(const char* label, uint32_t* mask) {
  const auto is_single = std::has_single_bit(*mask);
  const auto max_bit = std::bit_floor(*mask);
  const auto max_bit_pos = std::countr_zero(max_bit);
  std::string preview =
      ((is_single) ? std::to_string(max_bit_pos + 1) : "Multi Select");
  if ((*mask) == 0) preview = "None";
  if (ImGui::BeginCombo(label, preview.c_str())) {
    for (int n = 0; n < 32; n++) {
      if (n == 0) {
        const bool is_selected = (*mask) == 0;
        if (ImGui::Selectable("None", is_selected)) {
          (*mask) = 0;
        }
        continue;
      }

      const bool is_selected = ((*mask) & (1 << (n - 1)));
      if (ImGui::Selectable(std::to_string(n).c_str(), is_selected)) {
        if (is_selected) {
          (*mask) &= ~(1 << (n - 1));
        } else {
          (*mask) |= (1 << (n - 1));
        }
      } else {
      }
    }
    ImGui::EndCombo();
  }
}
void InspectorPanel::GenerateProperties(ObjectEntity& object) {
  properties_map_.clear();
  redraw_ = false;
  auto& registry = scene_context_->GetRegistry();
  for (const auto& storage : registry.storage()) {
    auto clazz = ComponentDB::GetClass(storage.first);
    if (!clazz) continue;
    auto data = storage.second.try_get(object.GetHandle());
    if (!data) continue;
    std::list<PropertyInfo> property_infos;
    object.GetClassPropertyList(storage.first, &property_infos);

    auto& properties = properties_map_[clazz->name];
    properties = std::vector<PropertyPtr>{};

    for (const auto& property_info : property_infos) {
      auto editor = inspector::MakeEditorProperty(data, property_info.type,
                                                  property_info.hint,
                                                  property_info.hint_name);
      if (!editor) continue;
      editor->SetObjectAndClassName(object, clazz->name);
      editor->SetPropertyName(property_info.name);
      editor->UpdateProperty();
      editor->Connect("PropertyChanged",
                      make_callable_function_pointer(
                          this, &InspectorPanel::PropertyChanged));

      properties.emplace_back(editor);
    }
  }
}

void InspectorPanel::PropertyChanged(const std::string& class_name,
                                     const std::string& property,
                                     const Variant& value) {
  ObjectEntity object = scene_context_->TryGetEntityWithUUID(select_item_);
  if (!object) return;
  const auto clazz = ComponentDB::GetClass(class_name);
  if (clazz->property_map.contains(property)) {
    if (clazz->property_map[property].hint == PropertyHint::kAsset) {
      redraw_ = true;
    }
  }
  object.TrySetProperty(class_name, property, value);
}

void InspectorPanel::OnRemovedComponent(uint32_t class_id) { redraw_ = true; }

void InspectorPanel::SetSceneContext(const Ref<Scene>& context) {
  scene_context_ = context;
}

void InspectorPanel::PickItem(const std::string& context, const uint64_t item) {
  if (context != "HierarchyPanel") return;
  if (select_item_ == item) return;
  select_item_ = item;

  ObjectEntity object = scene_context_->TryGetEntityWithUUID(select_item_);

  GenerateProperties(object);
}
}  // namespace base_engine::editor
