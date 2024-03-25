// @Skeleton.h
// @brief
// @author ICE
// @date 2024/02/27
//
// @details

#pragma once
#include <glm/glm.hpp>

#include "Asset.h"

namespace base_engine {

class MeshSource;

class Skeleton {
 public:
  static constexpr uint32_t kNullIndex = ~0;


  Skeleton() = default;
  Skeleton(uint32_t size);

  uint32_t AddBone(const std::string& name, uint32_t parentIndex,
                   const glm::mat4& transform);
  uint32_t GetBoneIndex(const std::string_view name) const;

  [[nodiscard]] uint32_t GetParentBoneIndex(const uint32_t boneIndex) const {
    return m_ParentBoneIndices[boneIndex];
  }

  [[nodiscard]] const std::vector<uint32_t>& GetParentBoneIndices() const {
    return m_ParentBoneIndices;
  }

  [[nodiscard]] uint32_t GetNumBones() const {
    return static_cast<uint32_t>(m_BoneNames.size());
  }

  [[nodiscard]] const std::string& GetBoneName(const uint32_t boneIndex) const {
    return m_BoneNames[boneIndex];
  }

  [[nodiscard]] const auto& GetBoneNames() const { return m_BoneNames; }

  [[nodiscard]] std::vector<glm::vec3> GetBoneTranslations() const {
    return m_BoneTranslations;
  }

  [[nodiscard]] std::vector<glm::quat> GetBoneRotations() const {
    return m_BoneRotations;
  }

  std::vector<glm::vec3> GetBoneScales() const { return m_BoneScales; }

  void SetBones(std::vector<std::string> boneNames,
                std::vector<uint32_t> parentBoneIndices,
                std::vector<glm::vec3> boneTranslations,
                std::vector<glm::quat> boneRotations,
                std::vector<glm::vec3> boneScales);

 private:
  std::vector<std::string> m_BoneNames;
  std::vector<uint32_t> m_ParentBoneIndices;

  // rest pose of skeleton. All in bone-local space (i.e.
  // translation/rotation/scale relative to parent)
  std::vector<glm::vec3> m_BoneTranslations;
  std::vector<glm::quat> m_BoneRotations;
  std::vector<glm::vec3> m_BoneScales;
};

class SkeletonAsset : public Asset {
 public:
  SkeletonAsset(const AssetHandle meshSource);
  SkeletonAsset(const Ref<MeshSource> meshSource);

  static AssetType GetStaticType() { return AssetType::kSkeleton; }
  virtual AssetType GetAssetType() const override { return GetStaticType(); }

  Ref<MeshSource> GetMeshSource() const;
  const Skeleton& GetSkeleton() const;

 private:
  Ref<MeshSource> m_MeshSource;
};

}  // namespace base_engine
