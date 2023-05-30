// @SceneSerializer.h
// @brief
// @author ICE
// @date 2023/05/15
//
// @details

#pragma once
#include <filesystem>

#include "ObjectEntity.h"
#include "Scene.h"
#include "yaml-cpp/node/node.h"

namespace YAML {
class Emitter;
}

namespace base_engine {
class SceneSerializer {
 public:
  explicit SceneSerializer(const Ref<Scene>& scene);

  /**
   * \brief Scene内のオブジェクトをシリアライズ化し、指定パスに
   * .bsceneファイルの保存を行う。\n
   * スクリプトのフィールドは、publicなどEditor側からアクセス可能な可視性レベルのものにたいしてシリアライズ化が行われる。
   * \param scene_file_path 保存先のファイルパス
   */
  void Serialize(const std::filesystem::path& scene_file_path);


  static void SerializeEntity(YAML::Emitter& out, ObjectEntity& entity);

  /**
   * \brief 指定パス先のファイルを読み取り、Sceneオブジェクトをロードする。
   * \param filepath 読み込み先のファイルパス
   * \return true :読み込み成功 \n false :読み込み失敗時
   */
  bool Deserialize(const std::filesystem::path& filepath);

  static void DeserializeEntities(YAML::Node& entities_node, const Ref<Scene>& scene);
 private:
  void SerializeToYAML(YAML::Emitter& out);
  bool DeserializeFromYAML(const std::string& yaml_str);

  Ref<Scene> scene_;

  /**
   * \brief メジャー番号'マイナー番号
   * メジャー番号：全体的な互換性が失うほどの変更
   * マイナー番号：自動での補完が可能な範囲の変更
   */
  struct Version {
    constexpr static size_t version = 0x00'01u;
    constexpr static size_t major_mask = 0xFF'00u;
    constexpr static size_t minor_mask = 0x00'FFu;

    constexpr static size_t GetVersion() { return version; }
    constexpr static size_t GetMajorVersion() { return version & major_mask; }
    constexpr static size_t GetMinorVersion() { return version & minor_mask; }
  };
};
}  // namespace base_engine