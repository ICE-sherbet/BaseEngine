#include "SceneSerializer.h"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <ranges>

#include "AssetManager.h"
#include "BodyMask.h"
#include "CSharpScriptEngine.h"
#include "MonoScriptUtilities.h"
#include "PhysicsObjectFactory.h"
#include "RigidBodyComponent.h"
#include "ShapeComponents.h"
#include "YAMLSerializeHelper.h"

namespace base_engine {

namespace internal {
using namespace component;

inline YAML::Emitter& operator<<(YAML::Emitter& emitter, const Vector2& v) {
  emitter << YAML::Flow;
  emitter << YAML::BeginMap << YAML::Key << "x" << YAML::Value << v.x
          << YAML::Key << "y" << YAML::Value << v.y << YAML::EndMap;
  return emitter;
}
inline YAML::Emitter& operator<<(YAML::Emitter& emitter, const Vector3& v) {
  emitter << YAML::Flow;
  emitter << YAML::BeginMap << YAML::Key << "x" << YAML::Value << v.x
          << YAML::Key << "y" << YAML::Value << v.y << YAML::Key << "z"
          << YAML::Value << v.z << YAML::EndMap;
  return emitter;
}
inline YAML::Emitter& operator<<(YAML::Emitter& emitter, const Vector4& q) {
  emitter << YAML::Flow;
  emitter << YAML::BeginMap << YAML::Key << "x" << YAML::Value << q.x
          << YAML::Key << "y" << YAML::Value << q.y << YAML::Key << "z"
          << YAML::Value << q.z << YAML::Key << "w" << YAML::Value << q.w
          << YAML::EndMap;
  return emitter;
}
inline YAML::Emitter& operator<<(YAML::Emitter& emitter, const Quaternion& q) {
  emitter << YAML::Flow;
  emitter << YAML::BeginMap << YAML::Key << "x" << YAML::Value << q.x
          << YAML::Key << "y" << YAML::Value << q.y << YAML::Key << "z"
          << YAML::Value << q.z << YAML::Key << "w" << YAML::Value << q.w
          << YAML::EndMap;
  return emitter;
}
inline YAML::Emitter& operator<<(YAML::Emitter& emitter, const UUID& id) {
  emitter << YAML::Value << static_cast<UUID::ValueType>(id);

  return emitter;
}

inline void SerializeTagComponent(YAML::Emitter& out, ObjectEntity& entity) {
  if (!entity.HasComponent<TagComponent>()) return;
  out << YAML::Key << "TagComponent";
  out << YAML::BeginMap;
  const auto& tag = entity.GetComponent<TagComponent>().tag;
  out << YAML::Key << "Tag" << YAML::Value << tag;
  out << YAML::EndMap;
}

inline void SerializeTransformComponent(YAML::Emitter& out,
                                        ObjectEntity& entity) {
  if (!entity.HasComponent<TransformComponent>()) return;
  out << YAML::Key << "TransformComponent";
  out << YAML::BeginMap;  // TransformComponent

  auto& transform = entity.GetComponent<TransformComponent>();
  out << YAML::Key << "Rotation" << YAML::Value << transform.GetLocalRotation();

  out << YAML::Key << "Position" << YAML::Value
      << transform.GetLocalTranslation();

  out << YAML::Key << "Scale" << YAML::Value << transform.GetLocalScale();

  out << YAML::EndMap;  // TransformComponent
}

inline void DeserializeTransformComponent(YAML::Node& node,
                                          ObjectEntity& entity) {
  auto transform_component = node["TransformComponent"];
  if (!transform_component) return;
  auto& transform = entity.GetComponent<TransformComponent>();

  transform.SetLocalRotation(
      transform_component["Rotation"].as<Quaternion>(Quaternion()));

  transform.SetLocalTranslation(
      transform_component["Position"].as<Vector3>(Vector3()));

  transform.SetLocalScale(transform_component["Scale"].as<Vector3>());
}

inline void SerializeHierarchyComponent(YAML::Emitter& out,
                                        ObjectEntity& entity) {
  if (!entity.HasComponent<HierarchyComponent>()) return;
  out << YAML::Key << "HierarchyComponent";
  out << YAML::BeginMap;  // HierarchyComponent

  const auto& hierarchy = entity.GetComponent<HierarchyComponent>();
  out << YAML::Key << "Parent" << YAML::Value << hierarchy.parent_handle;
  out << YAML::Key << "Children" << YAML::Value << hierarchy.children;
  out << YAML::EndMap;  // HierarchyComponent
}

inline void DeserializeHierarchyComponent(YAML::Node& node,
                                          ObjectEntity& entity) {
  auto hierarchy_component_node = node["HierarchyComponent"];
  if (!hierarchy_component_node) return;

  auto& hierarchy = entity.GetComponent<HierarchyComponent>();
  hierarchy.parent_handle = hierarchy_component_node["Parent"].as<UUID>();
  if (auto children = hierarchy_component_node["Children"]) {
    for (auto child : children) {
      auto child_handle = child.as<uint64_t>();
      hierarchy.children.emplace_back(child_handle);
    }
  }
}
inline void SerializeSpriteRendererComponent(YAML::Emitter& out,
                                             ObjectEntity& entity) {
  if (!entity.HasComponent<SpriteRendererComponent>()) return;
  out << YAML::Key << "SpriteRendererComponent";
  out << YAML::BeginMap;

  auto& sprite = entity.GetComponent<SpriteRendererComponent>();
  out << YAML::Key << "Sprite" << YAML::Value << sprite.texture;
  out << YAML::Key << "Color" << YAML::Value << sprite.color;
  out << YAML::Key << "Pivot" << YAML::Value << sprite.pivot;

  out << YAML::EndMap;
}
inline void DeserializeSpriteRendererComponent(YAML::Node& node,
                                               ObjectEntity& entity) {
  auto sprite_component_node = node["SpriteRendererComponent"];
  if (!sprite_component_node) return;

  auto& sprite = entity.AddComponent<SpriteRendererComponent>();
  sprite.texture = sprite_component_node["Sprite"].as<AssetHandle>();
  sprite.color = sprite_component_node["Color"].as<Vector4>();
  sprite.pivot = sprite_component_node["Pivot"].as<Vector2>();
  if (!AssetManager::IsAssetHandleValid(sprite.texture)) {
    BE_CORE_ERROR_TAG("Deserialize", "テクスチャアセットのUUIDが無効");
  };
}

inline void SerializeAudioComponent(YAML::Emitter& out, ObjectEntity& entity) {
  if (!entity.HasComponent<AudioComponent>()) return;
  out << YAML::Key << "AudioComponent";
  out << YAML::BeginMap;

  const auto& audio = entity.GetComponent<AudioComponent>();
  out << YAML::Key << "Source" << YAML::Value << audio.audio_source;
  out << YAML::EndMap;
}
inline void DeserializeAudioComponent(YAML::Node& node, ObjectEntity& entity) {
  auto audio_component_node = node["AudioComponent"];
  if (!audio_component_node) return;

  auto& audio = entity.AddComponent<AudioComponent>();
  audio.audio_source = audio_component_node["Source"].as<AssetHandle>();
  if (!AssetManager::IsAssetHandleValid(audio.audio_source)) {
    BE_CORE_ERROR_TAG("Deserialize", "オーディオアセットのUUIDが無効");
  };
}

inline void SerializeRigidBodyComponent(YAML::Emitter& out,
                                        ObjectEntity& entity) {
  if (!entity.HasComponent<physics::RigidBodyComponent>()) return;
  const auto& rigid = entity.GetComponent<physics::RigidBodyComponent>();
  out << YAML::Key << "RigidBodyComponent";
  out << YAML::BeginMap;
  out << YAML::Key << "Restitution" << YAML::Value << rigid.restitution;
  out << YAML::Key << "Mass" << YAML::Value << rigid.mass;
  out << YAML::EndMap;
}

inline physics::RigidBodyComponent DeserializeRigidBodyComponent(
    YAML::Node& node, ObjectEntity& entity) {
  auto rigid_component_node = node["RigidBodyComponent"];
  if (!rigid_component_node) return {0, 0, 0};
  return {rigid_component_node["Restitution"].as<float>(),
          rigid_component_node["Mass"].as<float>(), 0};
}
inline void SerializeBodyMask(YAML::Emitter& out, ObjectEntity& entity) {
  if (!entity.HasComponent<physics::BodyMask>()) return;
  out << YAML::Key << "BodyMaskComponent";
  out << YAML::BeginMap;
  const auto& mask = entity.GetComponent<physics::BodyMask>();
  out << YAML::Key << "ShapeType" << YAML::Value << mask.shape_type_id;
  out << YAML::Key << "BodyType" << YAML::Value << mask.tag_type_id;
  out << YAML::Key << "BodyMask" << YAML::Value << mask.body_mask;
  out << YAML::Key << "TargetMask" << YAML::Value << mask.target_mask;
  out << YAML::EndMap;
}
inline physics::BodyMask DeserializeBodyMask(YAML::Node& node,
                                             ObjectEntity& entity) {
  auto body_mask_node = node["BodyMaskComponent"];
  if (!body_mask_node) return {0, 0};
  return {body_mask_node["ShapeType"].as<int>(),
          body_mask_node["BodyType"].as<int>(),
          body_mask_node["BodyMask"].as<uint32_t>(),
          body_mask_node["TargetMask"].as<uint32_t>()};
}

inline void SerializeCircle(YAML::Emitter& out, ObjectEntity& entity) {
  if (!entity.HasComponent<physics::CircleShape>()) return;
  out << YAML::Key << "CircleComponent";
  out << YAML::BeginMap;
  const auto& [radius] = entity.GetComponent<physics::CircleShape>();
  out << YAML::Key << "Radius" << YAML::Value << radius;
  out << YAML::EndMap;
}
inline physics::CircleShape DeserializeCircle(YAML::Node& node,
                                              ObjectEntity& entity) {
  auto circle_node = node["CircleComponent"];
  if (!circle_node) return {0};
  return {circle_node["Radius"].as<float>()};
}
inline void SerializeE(YAML::Emitter& out, ObjectEntity& entity) {}

inline void DeserializeE(YAML::Node& node, ObjectEntity& entity) {}
inline void SerializeScriptComponent(YAML::Emitter& out, ObjectEntity& entity) {
  if (!entity.HasComponent<ScriptComponent>()) return;
  out << YAML::Key << "ScriptComponent";
  out << YAML::BeginMap;

  const auto& sc = entity.GetComponent<ScriptComponent>();

  const auto script_class =
      CSharpScriptEngine::GetInstance()->GetManagedClassById(
          CSharpScriptEngine::GetInstance()->GetScriptClassIdFromComponent(sc));
  out << YAML::Key << "ClassHandle" << YAML::Value << sc.script_class_handle;
  out << YAML::Key << "Name" << YAML::Value
      << (script_class ? script_class->full_name : "Null");

  if (!script_class->fields.empty()) {
    out << YAML::Key << "Fields" << YAML::Value;
    out << YAML::BeginMap;
    for (const auto field_id : script_class->fields) {
      auto field = CSharpScriptEngine::GetInstance()->GetFieldById(field_id);

      Ref<IFieldStorage> storage =
          CSharpScriptEngine::GetInstance()->GetFieldStorage(entity, field_id);
      auto obj = storage->GetValueVariant();
      const auto PrintYaml = [&out, &field](auto value) {
        out << YAML::Key << field->field_info.name << YAML::Value << value;
      };
      obj.Visit(PrintYaml);
    }

    out << YAML::EndMap;
  }

  out << YAML::EndMap;
}

inline void DeserializeScriptComponent(YAML::Node& node, ObjectEntity& entity) {
  auto script_node = node["ScriptComponent"];
  if (!script_node) return;
  auto asset_handle = script_node["script"].as<AssetHandle>();

  if (asset_handle == 0) {
    return;
  }

  auto& sc = entity.HasComponent<ScriptComponent>()
                 ? entity.GetComponent<ScriptComponent>()
                 : entity.AddComponent<ScriptComponent>(asset_handle);
  CSharpScriptEngine::GetInstance()->InitializeScriptEntity(entity);

  const auto script_class =
      CSharpScriptEngine::GetInstance()->GetManagedClassById(
          CSharpScriptEngine::GetInstance()->GetScriptClassIdFromComponent(sc));

  auto fields = script_node["Fields"];
  for (const auto field_id : script_class->fields) {
    auto field = CSharpScriptEngine::GetInstance()->GetFieldById(field_id);
    Variant object(field->field_info.type);
    object.Visit([&fields, &field](auto&& value) {
      if (!fields[field->field_info.name]) {
        return;
      }
      value = fields[field->field_info.name]
                  .as<std::remove_reference_t<decltype(value)>>();
    });
    Ref<IFieldStorage> storage =
        CSharpScriptEngine::GetInstance()->GetFieldStorage(entity, field_id);
    if (!storage) continue;
    storage->SetValueVariant(object);
  }
}

void DeserializePhysics(YAML::Node& entity, ObjectEntity& deserialized_entity) {
  const auto rigid = DeserializeRigidBodyComponent(entity, deserialized_entity);
  const auto mask = DeserializeBodyMask(entity, deserialized_entity);
  if (mask.shape_type_id == physics::CircleShape::Type()) {
    const auto [radius] = DeserializeCircle(entity, deserialized_entity);
    physics::PhysicsObjectFactory::CreateCircle(deserialized_entity, radius,
                                                rigid.mass, rigid.restitution);
    deserialized_entity.GetComponent<physics::BodyMask>().body_mask =
        mask.body_mask;
    deserialized_entity.GetComponent<physics::BodyMask>().target_mask =
        mask.target_mask;
  }
}

void DeserializeEntities(YAML::Node& entities_node, Ref<Scene> scene) {
  for (auto entity : entities_node) {
    const auto uuid = entity["Entity"].as<uint64_t>();

    std::string name;
    if (auto tag_component = entity["TagComponent"])
      name = tag_component["tag"].as<std::string>();

    ObjectEntity deserialized_entity = scene->CreateEntityWithUUID(uuid, name);

    auto& registry = scene->GetRegistry();
    std::vector<uint64_t> fix_component = {};

    for (const auto& node : entity) {
      auto class_name = node.first.as<std::string>();
      auto clazz = ComponentDB::GetClass(class_name);
      if (!clazz) {
        BE_CORE_INFO("ComponentDB has not class {0}", class_name);
        continue;
      }
      bool skip = false;
      for (const uint64_t component_id : fix_component) {
        if (component_id == clazz->id) {
          skip = true;
        }
      }
      if (skip) continue;

      if (clazz->id == ScriptComponent::_GetHash()) {
        std::list<PropertyInfo> property_infos;
        deserialized_entity.GetClassPropertyList(clazz->id, &property_infos);

        for (const auto& property_info : property_infos) {
          if (!node.second[property_info.name]) {
            BE_CORE_ASSERT("ファイル読み込みエラー プロパティ名 不一致");
            continue;
          }

          Variant s_value{property_info.type};
          s_value.Visit(
              [&node, &property_info, &s_value]<typename T>(T&& value) {
                value = node.second[property_info.name]
                            .as<std::remove_reference_t<T>>();
                s_value = Variant(value);
              });
          deserialized_entity.TrySetProperty(class_name, property_info,
                                             s_value);
        }
        CSharpScriptEngine::GetInstance()->InitializeScriptEntity(
            deserialized_entity);
      }
      std::list<PropertyInfo> property_infos;
      deserialized_entity.GetClassPropertyList(clazz->id, &property_infos);
      if (!registry.valid(clazz->id)) {
        registry.create_pool(clazz->id, clazz->registry_pool_factory);
        registry.storage(clazz->id)->try_emplace(deserialized_entity, false);
      }
      for (const auto& property_info : property_infos) {
        if (!node.second[property_info.name]) {
          BE_CORE_ASSERT("ファイル読み込みエラー プロパティ名 不一致");
          continue;
        }
        Variant s_value{property_info.type};
        s_value.Visit([&node, &property_info, &s_value]<typename T>(T&& value) {
          value =
              node.second[property_info.name].as<std::remove_reference_t<T>>();
          s_value = Variant(value);
        });
        deserialized_entity.TrySetProperty(class_name, property_info, s_value);
      }
    }
    // DeserializeScriptComponent(entity, deserialized_entity);
    DeserializeHierarchyComponent(entity, deserialized_entity);
    auto& transform = deserialized_entity.GetComponent<TransformComponent>();
    auto& hierarchy = deserialized_entity.GetComponent<HierarchyComponent>();

    transform.SetChildren(hierarchy.children);
    transform.SetParent(hierarchy.parent_handle);
  }
}
}  // namespace internal

SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : scene_(scene) {}

void SceneSerializer::Serialize(const std::filesystem::path& scene_file_path) {
  YAML::Emitter out;
  SerializeToYAML(out);

  std::ofstream fout(scene_file_path);
  fout << out.c_str();
}

void SceneSerializer::SerializeEntity(YAML::Emitter& out,
                                      ObjectEntity& entity) {
  using namespace component;
  using namespace internal;
  const UUID uuid = entity.GetComponent<IdComponent>().uuid;

  out << YAML::BeginMap;
  out << YAML::Key << "Entity";
  out << YAML::Value << uuid;

  SerializeHierarchyComponent(out, entity);
  auto& registry = entity.GetScene()->GetRegistry();
  std::vector<uint64_t> fix_component = {};
  for (const auto& storage : registry.storage()) {
    bool skip = false;
    for (const uint64_t component_id : fix_component) {
      if (component_id == storage.first) {
        skip = true;
      }
    }
    if (skip) continue;

    auto clazz = ComponentDB::GetClass(storage.first);
    if (!clazz) continue;
    auto data = storage.second.try_get(entity.GetHandle());
    if (!data) continue;
    std::list<PropertyInfo> property_infos;
    entity.GetClassPropertyList(storage.first, &property_infos);
    auto class_name = ComponentDB::GetClass(storage.first)->name;
    out << YAML::Key << class_name;
    out << YAML::BeginMap;

    for (const auto& property_info : property_infos) {
      Variant r_value;
      entity.TryGetProperty(class_name, property_info, r_value);
      r_value.Visit([&out, &property_info](auto&& value) {
        out << YAML::Key << property_info.name << YAML::Value << value;
      });
    }
    out << YAML::EndMap;
  }

  out << YAML::EndMap;
}

void SceneSerializer::SerializeToYAML(YAML::Emitter& out) {
  out << YAML::BeginMap;
  out << YAML::Key << "Version" << YAML::Value << Version::GetVersion();
  out << YAML::Key << "Scene";
  out << YAML::Value << static_cast<std::string>(scene_->GetName());

  out << YAML::Key << "Entities";
  out << YAML::Value << YAML::BeginSeq;

  std::map<UUID, becs::Entity> sorted_entity_map;
  for (const auto id_component_view =
           scene_->GetRegistry().view<component::IdComponent>();
       const auto entity : id_component_view)
    sorted_entity_map[id_component_view.get<component::IdComponent>(entity)
                          .uuid] = entity;

  for (const auto entity : sorted_entity_map | std::views::values) {
    ObjectEntity object_entity{entity, scene_.Raw()};
    SerializeEntity(out, object_entity);
  }

  out << YAML::EndSeq;

  out << YAML::EndMap;
}

bool SceneSerializer::Deserialize(const std::filesystem::path& filepath) {
  if (filepath.extension() != ".bscene") return false;

  const std::ifstream stream(filepath);
  std::stringstream str_stream;
  str_stream << stream.rdbuf();

  if (!DeserializeFromYAML(str_stream.str())) return false;
}

void SceneSerializer::DeserializeEntities(YAML::Node& entities_node,
                                          const Ref<Scene>& scene) {
  internal::DeserializeEntities(entities_node, scene);
}

bool SceneSerializer::DeserializeFromYAML(const std::string& yaml_str) {
  YAML::Node data = YAML::Load(yaml_str);

  if (!data["Scene"]) return false;
  if (!data["Version"]) {
    BE_CORE_ERROR("シーンファイルのフォーマットが正しくありません。");
    return false;
  }
  if (data["Version"].as<size_t>() != Version::GetVersion()) {
    BE_CORE_ERROR("バージョンとの互換性がないため読み込み失敗。");
    return false;
  }
  const auto& scene_name = data["Scene"].as<std::string>();
  BE_CORE_INFO("デシリアライズシーン {0}", scene_name.c_str());
  scene_->SetName(scene_name);

  if (auto entities = data["Entities"])
    internal::DeserializeEntities(entities, scene_);
}
}  // namespace base_engine
