#include "Prefab.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

#include "AssetImporter.h"
#include "BodyMask.h"
#include "BoundingBox.h"
#include "CSharpScriptEngine.h"
#include "RigidBodyComponent.h"
#include "SceneSerializer.h"
#include "ShapeComponents.h"
#include "VelocityComponent.h"

namespace base_engine {
Prefab::Prefab() { scene_ = Ref<Scene>::Create("Empty"); }

template <typename TComponent>
void CopyComponentIfExists(becs::Entity dst, becs::registry& dst_registry,
                           const becs::Entity src,
                           becs::registry& src_registry) {
  if (src_registry.any_of<TComponent>(src)) {
    auto src_component = src_registry.get<TComponent>(src);
    dst_registry.emplace_or_replace<TComponent>(dst, src_component);
  }
}

void Prefab::Create(const ObjectEntity entity, const bool serialize) {
  scene_ = Ref<Scene>::Create("Empty");
  entity_ = CreatePrefabFromEntity(entity);

  if (serialize) AssetImporter::Serialize(this);
}

ObjectEntity Prefab::CreatePrefabFromEntity(ObjectEntity entity) {
  using namespace component;
  ObjectEntity new_entity = scene_->CreateEntity();
  new_entity.AddComponent<PrefabComponent>(
      handle_, new_entity.GetComponent<IdComponent>().uuid);
  auto CopyComponentIfExistsFunc = [&entity, &new_entity,
                                    this]<typename Component>() {
    CopyComponentIfExists<Component>(new_entity, scene_->GetRegistry(), entity,
                                     entity.GetScene()->GetRegistry());
  };
  
  CopyComponentIfExistsFunc.operator()<TagComponent>();
  CopyComponentIfExistsFunc.operator()<TransformComponent>();
  CopyComponentIfExistsFunc.operator()<ScriptComponent>();
  CopyComponentIfExistsFunc.operator()<SpriteRendererComponent>();
  CopyComponentIfExistsFunc.operator()<physics::RigidBodyComponent>();
  CopyComponentIfExistsFunc.operator()<physics::VelocityComponent>();
  CopyComponentIfExistsFunc.operator()<physics::BodyMask>();
  CopyComponentIfExistsFunc.operator()<physics::Circle>();
  CopyComponentIfExistsFunc.operator()<physics::BoundingBox>();
  for (const auto child_id : entity.Children()) {
    ObjectEntity child_duplicate =
        CreatePrefabFromEntity(entity.GetScene()->GetEntityWithUUID(child_id));

    child_duplicate.SetParentUUID(new_entity.GetUUID());
    new_entity.Children().push_back(child_duplicate.GetUUID());
  }

  if (new_entity.HasComponent<ScriptComponent>())
    CSharpScriptEngine::GetInstance()->DuplicateScriptInstance(entity,
                                                               new_entity);

  return new_entity;
}

void PrefabSerializer::Serialize(const AssetMetadata& metadata,
                                 const Ref<Asset>& asset) const {
  const Ref<Prefab> prefab = asset.As<Prefab>();

  const std::string yaml_string = SerializeToYAML(prefab);

  std::ofstream fout(metadata.file_path);
  fout << yaml_string;
}

bool PrefabSerializer::TryLoadData(const AssetMetadata& metadata,
                                   Ref<Asset>& asset) const {
  const std::ifstream stream(metadata.file_path);
  if (!stream.is_open()) return false;

  std::stringstream str_stream;
  str_stream << stream.rdbuf();

  const auto prefab = Ref<Prefab>::Create();
  const bool success = DeserializeFromYAML(str_stream.str(), prefab);
  if (!success) return false;

  asset = prefab;
  asset->handle_ = metadata.handle;
  return true;
}

void PrefabSerializer::GetRecognizedExtensions(
    std::list<std::string>* extensions) const {
  extensions->emplace_back(".prefab");
}

std::string PrefabSerializer::GetAssetType(
    const std::filesystem::path& path) const {
  if (path.extension() == ".prefab") {
    return "Prefab";
  }
  return "";
}

std::string PrefabSerializer::SerializeToYAML(Ref<Prefab> prefab) const {
  YAML::Emitter out;

  out << YAML::BeginMap;
  out << YAML::Key << "Prefab";
  out << YAML::Value << YAML::BeginSeq;

  for (const auto view =
           prefab->scene_->GetRegistry().view<component::IdComponent>();
       const auto entity_id : view) {
    ObjectEntity entity = {entity_id, prefab->scene_.Raw()};
    if (!entity || !entity.HasComponent<component::IdComponent>()) continue;

    SceneSerializer::SerializeEntity(out, entity);
  }
  out << YAML::EndSeq;
  out << YAML::EndMap;

  return out.c_str();
}

bool PrefabSerializer::DeserializeFromYAML(const std::string& yaml_string,
                                           Ref<Prefab> prefab) const {
  YAML::Node data = YAML::Load(yaml_string);
  if (!data["Prefab"]) return false;

  YAML::Node prefab_node = data["Prefab"];
  SceneSerializer::DeserializeEntities(prefab_node, prefab->scene_);
  return true;
}
}  // namespace base_engine
