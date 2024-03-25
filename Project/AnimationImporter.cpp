#include "AnimationImporter.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <ranges>
#include <set>

#include "MeshImporterUtilities.h"

namespace base_engine::AnimationImporter {

static const uint32_t s_AnimationImportFlags =
    aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType |
    aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_OptimizeMeshes |
    aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights |
    aiProcess_GlobalScale | aiProcess_ValidateDataStructure;

class BoneHierarchy {
 public:
  explicit BoneHierarchy(const aiScene* scene) : scene_(scene){};

  void ExtractBones() {
    for (uint32_t mesh_index = 0; mesh_index < scene_->mNumMeshes;
         ++mesh_index) {
      const aiMesh* mesh = scene_->mMeshes[mesh_index];
      for (uint32_t bone_index = 0; bone_index < mesh->mNumBones;
           ++bone_index) {
        bones_.emplace(mesh->mBones[bone_index]->mName.C_Str());
      }
    }
  }
  void TraverseNode(const aiNode* node, Skeleton* skeleton) {
    if (bones_.contains(node->mName.C_Str())) {
      TraverseBone(node, skeleton, Skeleton::kNullIndex);
    } else {
      for (uint32_t node_index = 0; node_index < node->mNumChildren;
           ++node_index) {
        TraverseNode(node->mChildren[node_index], skeleton);
      }
    }
  }

  static void TraverseBone(const aiNode* node, Skeleton* skeleton,
                           const uint32_t parent_index) {
    const uint32_t bone_index = skeleton->AddBone(
        node->mName.C_Str(), parent_index,
        utilities::Mat4FromAIMatrix4x4(node->mTransformation));
    for (uint32_t node_index = 0; node_index < node->mNumChildren;
         ++node_index) {
      TraverseBone(node->mChildren[node_index], skeleton, bone_index);
    }
  }
  std::unique_ptr<Skeleton> CreateSkeleton() {
    if (!scene_) {
      return nullptr;
    }

    ExtractBones();
    if (bones_.empty()) {
      return nullptr;
    }

    auto skeleton = std::make_unique<Skeleton>(bones_.size());
    TraverseNode(scene_->mRootNode, skeleton.get());

    return skeleton;
  }

 private:
  std::set<std::string_view> bones_;
  const aiScene* scene_;
};

std::unique_ptr<Skeleton> ImportSkeleton(const std::string_view filename) {
  Assimp::Importer importer;
  const aiScene* scene =
      importer.ReadFile(filename.data(), s_AnimationImportFlags);
  return ImportSkeleton(scene);
}

std::unique_ptr<Skeleton> ImportSkeleton(const aiScene* scene) {
  BoneHierarchy bone_hierarchy(scene);
  return bone_hierarchy.CreateSkeleton();
}

void GetAnimationNames(const aiScene* scene, std::vector<std::string>* names) {
  if (scene) {
    names->reserve(names->capacity() + scene->mNumAnimations);
    for (size_t i = 0; i < scene->mNumAnimations; ++i) {
      if (scene->mAnimations[i]->mDuration > 0.0f) {
        names->emplace_back(scene->mAnimations[i]->mName.C_Str());
      } else {
        BE_CONSOLE_LOG_WARN(
            "Animation '{0}' duration is zero or negative.  This animation was "
            "ignored!",
            scene->mAnimations[i]->mName.C_Str());
      }
    }
  }
}

template <typename T>
struct KeyFrame {
  float FrameTime;
  T Value;
  KeyFrame(const float frameTime, const T& value)
      : FrameTime(frameTime), Value(value) {}
};

struct Channel {
  std::vector<KeyFrame<glm::vec3>> Translations;
  std::vector<KeyFrame<glm::quat>> Rotations;
  std::vector<KeyFrame<glm::vec3>> Scales;
  uint32_t Index;
};

static auto ImportChannels(const aiAnimation* anim, const Skeleton& skeleton,
                           const bool is_masked_root_motion,
                           const glm::vec3& root_translation_mask,
                           float root_rotation_mask) {
  std::vector<Channel> channels;

  std::unordered_map<std::string_view, uint32_t> bone_indices;
  std::unordered_set<uint32_t> root_bone_indices;
  for (uint32_t i = 0; i < skeleton.GetNumBones(); ++i) {
    bone_indices.emplace(skeleton.GetBoneName(i), i + 1);
    if (skeleton.GetParentBoneIndex(i) == Skeleton::kNullIndex)
      root_bone_indices.emplace(i + 1);
  }

  std::map<uint32_t, aiNodeAnim*> valid_channels;
  for (uint32_t channel_index = 0; channel_index < anim->mNumChannels;
       ++channel_index) {
    aiNodeAnim* node_anim = anim->mChannels[channel_index];
    if (auto it = bone_indices.find(node_anim->mNodeName.C_Str());
        it != bone_indices.end()) {
      valid_channels.emplace(it->second, node_anim);
    }
  }

  channels.resize(skeleton.GetNumBones() + 1);
  for (uint32_t bone_index = 1; bone_index < channels.size(); ++bone_index) {
    auto& [Translations, Rotations, Scales, Index] = channels[bone_index];
    Index = bone_index;
    if (auto valid_channel = valid_channels.find(bone_index);
        valid_channel != valid_channels.end()) {
      auto node_anim = valid_channel->second;

      // 最初と最後にキーを追加する可能性があるため２つ余分に確保する
      Translations.reserve(node_anim->mNumPositionKeys + 2);
      Rotations.reserve(node_anim->mNumRotationKeys + 2);
      Scales.reserve(node_anim->mNumScalingKeys + 2);

      for (uint32_t key_index = 0; key_index < node_anim->mNumPositionKeys;
           ++key_index) {
        aiVectorKey key = node_anim->mPositionKeys[key_index];
        float frame_time = std::clamp(
            static_cast<float>(key.mTime / anim->mDuration), 0.0f, 1.0f);
        if ((key_index == 0) && (frame_time > 0.0f)) {
          channels[bone_index].Translations.emplace_back(
              0.0f, glm::vec3{(key.mValue.x), (key.mValue.y), (key.mValue.z)});
        }
        Translations.emplace_back(
            frame_time,
            glm::vec3{(key.mValue.x), (key.mValue.y), (key.mValue.z)});
      }
      if (Translations.empty()) {
        BE_CORE_WARN_TAG("Animation",
                         "No translation track found for bone '{}'",
                         skeleton.GetBoneName(bone_index - 1));
        Translations = {{0.0f, glm::vec3{0.0f}}, {1.0f, glm::vec3{0.0f}}};
      } else if (Translations.back().FrameTime < 1.0f) {
        Translations.emplace_back(1.0f, Translations.back().Value);
      }
      for (uint32_t key_index = 0; key_index < node_anim->mNumRotationKeys;
           ++key_index) {
        aiQuatKey key = node_anim->mRotationKeys[key_index];
        float frame_time = std::clamp(
            static_cast<float>(key.mTime / anim->mDuration), 0.0f, 1.0f);

        if ((key_index == 0) && (frame_time > 0.0f)) {
          Rotations.emplace_back(
              0.0f, glm::quat{(key.mValue.w), (key.mValue.x), (key.mValue.y),
                              (key.mValue.z)});
        }
        Rotations.emplace_back(frame_time,
                               glm::quat{(key.mValue.w), (key.mValue.x),
                                         (key.mValue.y), (key.mValue.z)});
        BE_CORE_ASSERT(
            fabs(glm::length(channels[bone_index].Rotations.back().Value) -
                 1.0f) < 0.00001f);
      }
      if (Rotations.empty()) {
        BE_CORE_WARN_TAG("Animation", "No rotation track found for bone '{}'",
                         skeleton.GetBoneName(bone_index - 1));
        Rotations = {{0.0f, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}},
                     {1.0f, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}}};
      } else if (Rotations.back().FrameTime < 1.0f) {
        Rotations.emplace_back(1.0f, Rotations.back().Value);
      }
      for (uint32_t key_index = 0; key_index < node_anim->mNumScalingKeys;
           ++key_index) {
        aiVectorKey key = node_anim->mScalingKeys[key_index];
        float frame_time = std::clamp(
            static_cast<float>(key.mTime / anim->mDuration), 0.0f, 1.0f);
        if (key_index == 0 && frame_time > 0.0f) {
          Scales.emplace_back(
              0.0f, glm::vec3{(key.mValue.x), (key.mValue.y), (key.mValue.z)});
        }
        Scales.emplace_back(
            frame_time,
            glm::vec3{(key.mValue.x), (key.mValue.y), (key.mValue.z)});
      }
      if (Scales.empty()) {
        BE_CORE_WARN_TAG("Animation", "No scale track found for bone '{}'",
                         skeleton.GetBoneName(bone_index - 1));
        Scales = {{0.0f, glm::vec3{1.0f}}, {1.0f, glm::vec3{1.0f}}};
      } else if (Scales.back().FrameTime < 1.0f) {
        Scales.emplace_back(1.0f, channels[bone_index].Scales.back().Value);
      }
    } else {
      BE_CORE_WARN_TAG("Animation", "No animation tracks found for bone '{}'",
                       skeleton.GetBoneName(bone_index - 1));
      Translations = {{0.0f, glm::vec3{0.0f}}, {1.0f, glm::vec3{0.0f}}};
      Rotations = {{0.0f, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}},
                   {1.0f, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}}};
      Scales = {{0.0f, glm::vec3{1.0f}}, {1.0f, glm::vec3{1.0f}}};
    }
  }

  BE_CORE_ASSERT(!root_bone_indices.contains(1));
  {
    auto& [Translations, Rotations, Scales, Index] = channels[0];
    Index = 0;
    if (is_masked_root_motion) {
      for (auto& translation : channels[1].Translations) {
        Translations.emplace_back(translation.FrameTime,
                                  translation.Value * root_translation_mask);
        translation.Value *= (glm::vec3(1.0f) - root_translation_mask);
      }
      for (auto& rotation : channels[1].Rotations) {
        if (root_rotation_mask > 0.0f) {
          auto angle_y = utilities::AngleAroundYAxis(rotation.Value);
          Rotations.emplace_back(rotation.FrameTime,
                                 glm::quat{glm::cos(angle_y * 0.5f),
                                           glm::vec3{0.0f, 1.0f, 0.0f} *
                                               glm::sin(angle_y * 0.5f)});
          rotation.Value =
              glm::conjugate(glm::quat(
                  glm::cos(angle_y * 0.5f),
                  glm::vec3{0.0f, 1.0f, 0.0f} * glm::sin(angle_y * 0.5f))) *
              rotation.Value;
        } else {
          Rotations.emplace_back(rotation.FrameTime,
                                 glm::quat{1.0f, 0.0f, 0.0f, 0.0f});
        }
      }
    } else {
      Translations = channels[1].Translations;
      Rotations = channels[1].Rotations;
      channels[1].Translations = {{0.0f, glm::vec3{0.0f}},
                                  {1.0f, glm::vec3{0.0f}}};
      channels[1].Rotations = {{0.0f, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}},
                               {1.0f, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}}};
    }
    Scales = {{0.0f, glm::vec3{1.0f}}, {1.0f, glm::vec3{1.0f}}};
  }

  {
    auto& [Translations, Rotations, Scales, Index] = channels[0];
    for (const auto root_bone_index : root_bone_indices) {
      if (root_bone_index != 1) {
        for (auto& translation : channels[root_bone_index].Translations) {
          for (size_t root_motion_frame = 0;
               root_motion_frame < Translations.size() - 1;
               ++root_motion_frame) {
            if (Translations[root_motion_frame + 1].FrameTime >=
                translation.FrameTime) {
              const float alpha =
                  (translation.FrameTime -
                   Translations[root_motion_frame].FrameTime) /
                  (Translations[root_motion_frame + 1].FrameTime -
                   Translations[root_motion_frame].FrameTime);
              translation.Value -=
                  glm::mix(Translations[root_motion_frame].Value,
                           Translations[root_motion_frame + 1].Value, alpha);
              break;
            }
          }
        }

        for (auto& rotation : channels[root_bone_index].Rotations) {
          for (size_t root_motion_frame = 0;
               root_motion_frame < Rotations.size() - 1; ++root_motion_frame) {
            if (Rotations[root_motion_frame + 1].FrameTime >=
                rotation.FrameTime) {
              const float alpha = (rotation.FrameTime -
                                   Rotations[root_motion_frame].FrameTime) /
                                  (Rotations[root_motion_frame + 1].FrameTime -
                                   Rotations[root_motion_frame].FrameTime);
              rotation.Value = glm::normalize(
                  glm::conjugate(glm::slerp(
                      Rotations[root_motion_frame].Value,
                      Rotations[root_motion_frame + 1].Value, alpha)) *
                  rotation.Value);
              break;
            }
          }
        }
      }
    }
  }
  return channels;
}

static auto ConcatenateChannelsAndSort(const std::vector<Channel>& channels) {
  uint32_t num_translations = 0;
  uint32_t num_rotations = 0;
  uint32_t num_scales = 0;

  for (const auto& [Translations, Rotations, Scales, Index] : channels) {
    num_translations += Translations.size();
    num_rotations += Rotations.size();
    num_scales += Scales.size();
  }

  std::vector<std::pair<float, TranslationKey>> translation_keys_temp;
  std::vector<std::pair<float, RotationKey>> rotation_keys_temp;
  std::vector<std::pair<float, ScaleKey>> scale_keys_temp;
  translation_keys_temp.reserve(num_translations);
  rotation_keys_temp.reserve(num_rotations);
  scale_keys_temp.reserve(num_scales);
  for (const auto& [Translations, Rotations, Scales, Index] : channels) {
    float prev_frame_time = -1.0f;
    for (const auto& translation : Translations) {
      translation_keys_temp.emplace_back(
          prev_frame_time,
          TranslationKey{translation.FrameTime, Index, translation.Value});
      prev_frame_time = translation.FrameTime;
    }

    prev_frame_time = -1.0f;
    for (const auto& rotation : Rotations) {
      rotation_keys_temp.emplace_back(
          prev_frame_time,
          RotationKey{rotation.FrameTime, Index, rotation.Value});
      prev_frame_time = rotation.FrameTime;
    }

    prev_frame_time = -1.0f;
    for (const auto& scale : Scales) {
      scale_keys_temp.emplace_back(
          prev_frame_time, ScaleKey{scale.FrameTime, Index, scale.Value});
      prev_frame_time = scale.FrameTime;
    }
  }
  std::ranges::sort(translation_keys_temp, [](const auto& a, const auto& b) {
    return (a.first < b.first) ||
           ((a.first == b.first) && a.second.Track < b.second.Track);
  });
  std::ranges::sort(rotation_keys_temp, [](const auto& a, const auto& b) {
    return (a.first < b.first) ||
           ((a.first == b.first) && a.second.Track < b.second.Track);
  });
  std::ranges::sort(scale_keys_temp, [](const auto& a, const auto& b) {
    return (a.first < b.first) ||
           ((a.first == b.first) && a.second.Track < b.second.Track);
  });

  return std::tuple{translation_keys_temp, rotation_keys_temp, scale_keys_temp};
}

static auto ExtractKeys(
    const std::vector<std::pair<float, TranslationKey>>& translations,
    const std::vector<std::pair<float, RotationKey>>& rotations,
    const std::vector<std::pair<float, ScaleKey>>& scales) {
  std::vector<TranslationKey> translation_key_frames;
  std::vector<RotationKey> rotation_key_frames;
  std::vector<ScaleKey> scale_key_frames;
  translation_key_frames.reserve(translations.size());
  rotation_key_frames.reserve(rotations.size());
  scale_key_frames.reserve(scales.size());
  for (const auto& val : translations | std::views::values) {
    translation_key_frames.emplace_back(val);
  }
  for (const auto& val : rotations | std::views::values) {
    rotation_key_frames.emplace_back(val);
  }
  for (const auto& scale : scales | std::views::values) {
    scale_key_frames.emplace_back(scale);
  }

  return std::tuple{translation_key_frames, rotation_key_frames,
                    scale_key_frames};
}

std::unique_ptr<Animation> ImportAnimation(
    const aiScene* scene, const uint32_t animation_index,
    const Skeleton& skeleton, const bool is_masked_root_motion,
    const glm::vec3& root_translation_mask, const float root_rotation_mask) {
  if (!scene) {
    return nullptr;
  }

  if (animation_index >= scene->mNumAnimations) {
    return nullptr;
  }

  const aiAnimation* anim = scene->mAnimations[animation_index];
  const auto channels =
      ImportChannels(anim, skeleton, is_masked_root_motion,
                     root_translation_mask, root_rotation_mask);
  auto [translationKeysTemp, rotationKeysTemp, scaleKeysTemp] =
      ConcatenateChannelsAndSort(channels);
  auto [translationKeys, rotationKeys, scaleKeys] =
      ExtractKeys(translationKeysTemp, rotationKeysTemp, scaleKeysTemp);

  double sampling_rate = anim->mTicksPerSecond;
  if (sampling_rate < 0.0001) {
    sampling_rate = 1.0;
  }

  auto animation = std::make_unique<Animation>(
      static_cast<float>(anim->mDuration / sampling_rate), channels.size());
  animation->SetKeys(std::move(translationKeys), std::move(rotationKeys),
                     std::move(scaleKeys));
  return animation;
}

}  // namespace base_engine::AnimationImporter
