// @MeshComponent.h
// @brief
// @author ICE
// @date 2024/03/16
//
// @details

#pragma once

#include "ComponentDB.h"
#include "ComponentProperty.h"
#include "MaterialAsset.h"

namespace base_engine::component {

struct MeshComponent {
  BE_COMPONENT(MeshComponent)

  AssetHandle mesh = kNullUuid;
  uint32_t sub_mesh_index = 0;
  bool visible = true;
  Ref<MaterialTable> material_table = Ref<MaterialTable>::Create();

  static void _Bind();
};

constexpr size_t kMeshComponentSize = sizeof(MeshComponent);
}  // namespace base_engine::component
