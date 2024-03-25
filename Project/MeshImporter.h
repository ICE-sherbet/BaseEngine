// @MeshImporter.h
// @brief
// @author ICE
// @date 2024/03/08
//
// @details

#pragma once
#include <filesystem>

#include "MeshSource.h"
#include "Ref.h"

namespace base_engine {
class MeshImporter {
 public:
  explicit MeshImporter(std::filesystem::path path);

  Ref<MeshSource> ImportToMeshSource();
  bool ImportSkeleton(std::unique_ptr<Skeleton>& skeleton) const;
  bool ImportAnimation(const uint32_t animation_index, const Skeleton& skeleton,
                       const bool is_masked_root_motion,
                       const glm::vec3& root_translation_mask,
                       float root_rotation_mask,
                       std::unique_ptr<Animation>& animation) const;
  [[nodiscard]] bool IsCompatibleSkeleton(const uint32_t animation_index,
                                          const Skeleton& skeleton) const;
  [[nodiscard]] uint32_t GetAnimationCount() const;

 private:
  static void TraverseNodes(Ref<MeshSource> mesh_source, void* assimp_node,
                            uint32_t node_index,
                            const glm::mat4& parent_transform = glm::mat4(1.0f),
                            uint32_t level = 0);

 private:
  const std::filesystem::path path_;
};
}  // namespace base_engine
