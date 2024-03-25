#include "Skeleton.h"

#include <glm/detail/type_quat.hpp>

#include "AssetManager.h"
#include "MathUtilities.h"
#include "MeshSource.h"

namespace base_engine {
Skeleton::Skeleton(uint32_t size) {
  m_BoneNames.reserve(size);
  m_ParentBoneIndices.reserve(size);
  m_BoneTranslations.reserve(size);
  m_BoneRotations.reserve(size);
  m_BoneScales.reserve(size);
}

uint32_t Skeleton::AddBone(const std::string& name, uint32_t parentIndex,
                           const glm::mat4& transform) {
  const auto index = m_BoneNames.size();
  m_BoneNames.emplace_back(name);
  m_ParentBoneIndices.emplace_back(parentIndex);
  m_BoneTranslations.emplace_back();
  m_BoneRotations.emplace_back();
  m_BoneScales.emplace_back();
  math::DecomposeTransform(transform, m_BoneTranslations.back(),
                           m_BoneRotations.back(), m_BoneScales.back());

  return index;
}

uint32_t Skeleton::GetBoneIndex(const std::string_view name) const {
  for (size_t i = 0; i < m_BoneNames.size(); ++i) {
    if (m_BoneNames[i] == name) {
      return static_cast<uint32_t>(i);
    }
  }
  return Skeleton::kNullIndex;
}

void Skeleton::SetBones(std::vector<std::string> boneNames,
                        std::vector<uint32_t> parentBoneIndices,
                        std::vector<glm::vec3> boneTranslations,
                        std::vector<glm::quat> boneRotations,
                        std::vector<glm::vec3> boneScales) {
  BE_CORE_ASSERT(parentBoneIndices.size() == boneNames.size());
  BE_CORE_ASSERT(boneTranslations.size() == boneNames.size());
  BE_CORE_ASSERT(boneRotations.size() == boneNames.size());
  BE_CORE_ASSERT(boneScales.size() == boneNames.size());
  m_BoneNames = std::move(boneNames);
  m_ParentBoneIndices = std::move(parentBoneIndices);
  m_BoneTranslations = std::move(boneTranslations);
  m_BoneRotations = std::move(boneRotations);
  m_BoneScales = std::move(boneScales);
}

SkeletonAsset::SkeletonAsset(const AssetHandle meshSource)
    : m_MeshSource(AssetManager::GetAsset<MeshSource>(meshSource)) {}

SkeletonAsset::SkeletonAsset(const Ref<MeshSource> meshSource)
    : m_MeshSource(meshSource) {}

Ref<MeshSource> SkeletonAsset::GetMeshSource() const { return m_MeshSource; }

const Skeleton& SkeletonAsset::GetSkeleton() const {
  BE_CORE_ASSERT(m_MeshSource && m_MeshSource->HasSkeleton());
  return m_MeshSource->GetSkeleton();
}
}  // namespace base_engine
