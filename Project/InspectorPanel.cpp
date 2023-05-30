#include "InspectorPanel.h"

#include <numbers>

#include "BodyMask.h"
#include "CSharpScriptEngine.h"
#include "ImGuiAssetHelper.h"
#include "ImGuiVariantHelper.h"
#include "MonoScriptUtilities.h"
#include "PhysicsObjectFactory.h"
#include "RigidBodyComponent.h"
#include "SelectManager.h"
#include "ShapeComponents.h"
#include "imgui.h"

namespace base_engine::editor {

void InspectorPanel::Initialize(const Ref<Scene>& context) {
  SetSceneContext(context);
  SelectManager::Instance()->Connect(
      "SelectItem",
      make_callable_function_pointer(this, &InspectorPanel::PickItem));
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
    SelectManager::Instance()->SelectItem(0);
    return;
  }
  ImGui::BeginGroup();
  RenderTag(object);
  RenderTransform(object);
  RenderSprite(object);
  RenderScript(object);
  RenderRigidBody(object);
  RenderCircleShape(object);
  ImGui::EndGroup();
  RenderAddComponentButton(object);
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
    AddComponentButton<SpriteRendererComponent>("SpriteRenderer", object);
    AddComponentButton<ScriptComponent>("Script", object);
    AddComponentsButtonEvent<physics::Circle>(
        "Circle Shape", object,
        [](ObjectEntity& object, std::tuple<physics::Circle>) {
          physics::PhysicsObjectFactory::CreateCircle(object, 1, 1, 1);
        });
    ImGui::EndPopup();
  }
}

void InspectorPanel::RenderTag(ObjectEntity& object) {
  if (ImGui::CollapsingHeader("Name", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::BeginGroup();
    auto&& tag = object.GetComponent<component::TagComponent>();
    char buffer[256] = {};
    strcpy_s(buffer, sizeof(buffer), tag.tag.c_str());
    if (ImGui::InputText("##Name", buffer, sizeof(buffer))) {
      tag.tag = std::string(buffer);
    }
    ImGui::EndGroup();
  }
}

void InspectorPanel::RenderTransform(ObjectEntity& object) {
  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::BeginGroup();

    const ImVec2 cursor = ImGui::GetCursorPos();
    // ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 4));

    auto&& transform = object.GetComponent<component::TransformComponent>();

    auto pos = transform.GetLocalTranslation();
    ImGui::DragFloat3("position", pos.fv, 1,
                      -(std::numeric_limits<float>::max)(),
                      (std::numeric_limits<float>::max)());

    transform.SetLocalTranslation(pos);

    auto rotate =
        transform.GetLocalRotationEuler() / std::numbers::pi_v<float> * 180.0f;
    ImGui::DragFloat3("rotate", rotate.fv, 1,
                      -(std::numeric_limits<float>::max)(),
                      (std::numeric_limits<float>::max)());
    transform.SetLocalRotationEuler(rotate * std::numbers::pi_v<float> /
                                    180.0f);

    ImGui::EndGroup();
  }
}

void InspectorPanel::RenderSprite(ObjectEntity& object) {
  if (!object.HasComponent<component::SpriteRendererComponent>()) return;

  if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::BeginGroup();
    auto& sprite_component =
        object.GetComponent<component::SpriteRendererComponent>();
    ImGuiHelper::AssetReferenceField("Texture", &sprite_component.texture,
                                     AssetType::kTexture);

    ImGui::ColorEdit4(
        "Color", sprite_component.color.fv,
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
    ImGui::EndGroup();
  }
}

void InspectorPanel::RenderScript(ObjectEntity& object) {
  if (!object.HasComponent<component::ScriptComponent>()) return;

  if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::BeginGroup();
    auto&& sc = object.GetComponent<component::ScriptComponent>();

    if (ImGuiHelper::AssetReferenceField("Script", &sc.script_class_handle,
                                         AssetType::kScript)) {
      CSharpScriptEngine::GetInstance()->InitializeScriptEntity(object);
    }

    const auto script_class =
        CSharpScriptEngine::GetInstance()->GetManagedClassById(
            CSharpScriptEngine::GetInstance()->GetScriptClassIdFromComponent(
                sc));
    if (script_class) {
      if (!script_class->fields.empty()) {
        for (const auto field_id : script_class->fields) {
          const auto field =
              CSharpScriptEngine::GetInstance()->GetFieldById(field_id);
          auto storage = CSharpScriptEngine::GetInstance()->GetFieldStorage(
              object, field_id);
          if (storage == nullptr) continue;
          Variant variant = storage->GetValueVariant();
          if (ImGuiHelper::VariantField(field->field_info.name, &variant))
            storage->SetValueVariant(variant);

        }
      }
    }
    ImGui::EndGroup();
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
void InspectorPanel::RenderRigidBody(ObjectEntity& object) {
  if (!object.HasComponent<physics::RigidBodyComponent>()) return;
  if (!ImGui::CollapsingHeader("RigidBody", ImGuiTreeNodeFlags_DefaultOpen))
    return;

  auto& rigid = object.GetComponent<physics::RigidBodyComponent>();
  auto& mask = object.GetComponent<physics::BodyMask>();
  ImGui::Text("Collision Filter");
  FilterCombo("Body Mask", &mask.body_mask);
  FilterCombo("Target Mask", &mask.target_mask);
}

void InspectorPanel::RenderCircleShape(ObjectEntity& object) {
  if (!object.HasComponent<physics::Circle>()) return;
  if (!ImGui::CollapsingHeader("Circle Shape", ImGuiTreeNodeFlags_DefaultOpen))
    return;

  auto& circle = object.GetComponent<physics::Circle>();
  ImGui::InputFloat("Radius", &circle.radius);
}

void InspectorPanel::SetSceneContext(const Ref<Scene>& context) {
  scene_context_ = context;
}

void InspectorPanel::PickItem(uint64_t item) { select_item_ = item; }
}  // namespace base_engine::editor
