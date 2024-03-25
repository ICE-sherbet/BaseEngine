// @BottomLevelAS.h
// @brief
// @author ICE
// @date 2024/03/15
//
// @details

#pragma once
#include <vulkan/vulkan.h>

#include "Mesh.h"
#include "VulkanBuffer.h"

namespace base_engine {

class BottomLevelAS : public RefCounted {
 public:
  struct BLASBuildData {
    std::vector<VkAccelerationStructureGeometryKHR> asGeometry;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> asBuildOffsetInfo;
    VkBuildAccelerationStructureFlagsKHR flags{0};
  };

  BottomLevelAS(VkCommandBuffer cmd, Ref<MeshSource> mesh);
  ~BottomLevelAS(){};

  operator VkAccelerationStructureKHR() const { return m_structure; }

 private:
  VkAccelerationStructureKHR m_structure;
  Ref<VulkanBuffer> m_structureMemory;
  Ref<VulkanBuffer> m_scratch;
};
}  // namespace base_engine
