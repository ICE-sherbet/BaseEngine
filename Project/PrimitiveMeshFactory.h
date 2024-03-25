// @PrimitiveMeshFactory.h
// @brief
// @author ICE
// @date 2024/03/11
// 
// @details

#pragma once
#include <glm/vec3.hpp>

#include "Asset.h"

namespace base_engine
{
class PrimitiveMeshFactory
{
public:
  PrimitiveMeshFactory() = delete;
  PrimitiveMeshFactory(const PrimitiveMeshFactory&) = delete;

  static AssetHandle CreateBox(const glm::vec3& size);
  static AssetHandle CreateSphere(const float radius, const int latitude_bands = 30, const int longitude_bands = 30);
  static AssetHandle CreateCapsule(const float radius, const float height);
};
}
