// @AnimationImporter.h
// @brief
// @author ICE
// @date 2024/03/09
//
// @details

#pragma once
#include <assimp/scene.h>

#include <memory>
#include <string_view>

#include "Animation.h"
#include "Skeleton.h"

namespace base_engine::AnimationImporter {

std::unique_ptr<Skeleton> ImportSkeleton(const std::string_view filename);
std::unique_ptr<Skeleton> ImportSkeleton(const aiScene* scene);

void GetAnimationNames(const aiScene* scene, std::vector<std::string>* names);
std::unique_ptr<Animation> ImportAnimation(
    const aiScene* scene, const uint32_t animation_index,
    const Skeleton& skeleton, const bool is_masked_root_motion,
    const glm::vec3& root_translation_mask, const float root_rotation_mask);

}  // namespace base_engine::AnimationImporter
