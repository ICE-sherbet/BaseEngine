// @TopLevelAS.h
// @brief
// @author ICE
// @date 2024/03/15
//
// @details

#pragma once
#include <vulkan/vulkan.h>

#include "Scene.h"
#include "VulkanBuffer.h"

namespace base_engine {
struct ObjInstance {
  VkTransformMatrixKHR matrix;
  uint32_t instanceId;
  AssetHandle mesh;

};


class TopLevelAS : public RefCounted {
 public:
  TopLevelAS(const VkCommandBuffer& cmd,
             const std::vector<VkAccelerationStructureInstanceKHR>& instances);

  operator VkAccelerationStructureKHR() const { return m_structure; }

  Ref<VulkanBuffer> instanceOffsetTable() const {
    return m_instanceOffsetTable;
  }

  const VkWriteDescriptorSetAccelerationStructureKHR& descriptorInfo() const {
    return m_descriptorInfo;
  }

 private:
  VkWriteDescriptorSetAccelerationStructureKHR m_descriptorInfo = {};

  VkAccelerationStructureKHR m_structure;
  Ref<VulkanBuffer> m_structureMemory;
  Ref<VulkanBuffer> m_instances;
  Ref<VulkanBuffer> m_scratch;

  Ref<VulkanBuffer> m_instanceOffsetTable;
};

struct InstanceOffsetTableEntry {
  using uint = uint32_t;
  uint VertexBufferOffset;
  uint IndexBufferOffset;
  uint MaterialBufferOffset;
};
}  // namespace base_engine
