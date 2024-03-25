#include "BottomLevelAS.h"

#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanIndexBuffer.h"
#include "VulkanVertexBuffer.h"

namespace base_engine {
BottomLevelAS::BottomLevelAS(VkCommandBuffer cmd, Ref<MeshSource> mesh) {

  Ref<VulkanDevice> device = VulkanContext::Get()->GetCurrentDevice();

  const VkDeviceAddress vertex_address = GetBufferDeviceAddress(
      device.Raw(),
      mesh->GetVertexBuffer().As<VulkanVertexBuffer>()->GetVulkanBuffer());
  const VkDeviceAddress index_address = GetBufferDeviceAddress(
      device.Raw(),
      mesh->GetIndexBuffer().As<VulkanIndexBuffer>()->GetVulkanBuffer());
  VkAccelerationStructureGeometryTrianglesDataKHR triangles{
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR};
  triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
  triangles.vertexData.deviceAddress = vertex_address;
  triangles.vertexStride = sizeof(Vertex);
  triangles.indexType = VK_INDEX_TYPE_UINT32;
  triangles.indexData.deviceAddress = index_address;
  triangles.transformData = {};
  triangles.maxVertex = mesh->GetVertices().size();

  VkAccelerationStructureGeometryKHR asGeom{
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
  asGeom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
  asGeom.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
  asGeom.geometry.triangles = triangles;

  VkAccelerationStructureBuildRangeInfoKHR offset;
  offset.firstVertex = 0;
  offset.primitiveCount = mesh->GetIndices().size() / 3;
  offset.primitiveOffset = 0;
  offset.transformOffset = 0;

  BLASBuildData input;
  input.asGeometry.emplace_back(asGeom);
  input.asBuildOffsetInfo.emplace_back(offset);
  input.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DATA_ACCESS_KHR;
  VkAccelerationStructureBuildGeometryInfoKHR buildInfo{
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
  {
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.flags =
        input.flags |
        VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR |
        VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
    buildInfo.geometryCount = static_cast<uint32_t>(input.asGeometry.size());
    buildInfo.pGeometries = input.asGeometry.data();
    std::vector<uint32_t> maxPrimCount(input.asBuildOffsetInfo.size());
    for (auto tt = 0; tt < input.asBuildOffsetInfo.size(); tt++)
      maxPrimCount[tt] = input.asBuildOffsetInfo[tt].primitiveCount;

    VkAccelerationStructureBuildSizesInfoKHR sizeInfo;
    vkGetAccelerationStructureBuildSizesKHR(
        device->GetVulkanDevice(),
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
        maxPrimCount.data(), &sizeInfo);

    VkAccelerationStructureCreateInfoKHR createInfo{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    createInfo.size = sizeInfo.accelerationStructureSize;
    // Allocating the buffer to hold the acceleration structure
    m_structureMemory = Ref<VulkanBuffer>::Create(
        "BLAS Scratch Memory", createInfo.size,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,false);

    createInfo.buffer = m_structureMemory->operator VkBuffer();
    vkCreateAccelerationStructureKHR(device->GetVulkanDevice(), &createInfo,
                                     nullptr,
                                     &buildInfo.dstAccelerationStructure);
    m_structure = buildInfo.dstAccelerationStructure;
    m_scratch = Ref<VulkanBuffer>::Create(
        "BLAS Scratch", sizeInfo.accelerationStructureSize,
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
    VkBufferDeviceAddressInfo bufferInfo{
        VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr,
        m_scratch->operator VkBuffer()};
    VkDeviceAddress scratchAddress =
        vkGetBufferDeviceAddress(device->GetVulkanDevice(), &bufferInfo);
    buildInfo.scratchData.deviceAddress = scratchAddress;
  }
  auto buildOffset = input.asBuildOffsetInfo.data();
  vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &buildOffset);
}
}  // namespace base_engine
