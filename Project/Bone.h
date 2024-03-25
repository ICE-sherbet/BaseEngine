// @Bone.h
// @brief
// @author ICE
// @date 2024/02/26
//
// @details

#pragma once
#include <glm/glm.hpp>

#include "Log.h"

namespace base_engine {
struct BoneInfo {
  glm::mat4 SubMeshInverseTransform;
  glm::mat4 InverseBindPose;
  uint32_t SubMeshIndex;
  uint32_t BoneIndex;

  BoneInfo() = default;
  BoneInfo(const glm::mat4& sub_mesh_inverse_transform, const glm::mat4& inverse_bind_pose,
           uint32_t sub_mesh_index, const uint32_t bone_index)
      : SubMeshInverseTransform(sub_mesh_inverse_transform),
        InverseBindPose(inverse_bind_pose),
        SubMeshIndex(sub_mesh_index),
        BoneIndex(bone_index) {}
};

struct BoneInfluence {
  uint32_t BoneInfoIndices[4] = {0, 0, 0, 0};
  float Weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};

  void AddBoneData(uint32_t bone_info_index, float weight) {
    if (weight < 0.0f || weight > 1.0f) {
      BE_CORE_WARN(
          "Vertex bone weight is out of range. We will clamp it to [0, 1] "
          "(BoneID={0}, Weight={1})",
          bone_info_index, weight);
      weight = std::clamp(weight, 0.0f, 1.0f);
    }
    if (weight > 0.0f) {
      for (size_t i = 0; i < 4; i++) {
        if (Weights[i] == 0.0f) {
          BoneInfoIndices[i] = bone_info_index;
          Weights[i] = weight;
          return;
        }
      }

      BE_CORE_WARN(
          "Vertex has more than four bones affecting it, extra bone influences "
          "will be discarded (BoneID={0}, Weight={1})",
          bone_info_index, weight);
    }
  }

  void NormalizeWeights() {
    float sum_weights = 0.0f;
    for (const float weight : Weights)
    {
      sum_weights += weight;
    }
    if (sum_weights > 0.0f) {
      for (size_t i = 0; i < 4; i++) {
        Weights[i] /= sum_weights;
      }
    }
  }
};
}  // namespace base_engine