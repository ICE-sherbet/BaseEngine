#include "TopLevelAS.h"

#include "AssetManager.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"

namespace base_engine {

TopLevelAS::TopLevelAS(
    const VkCommandBuffer& cmd,
    const std::vector<VkAccelerationStructureInstanceKHR>& instance) {
  // Wraps a device pointer to the above uploaded instances.
  VkAccelerationStructureGeometryInstancesDataKHR instancesVk{
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR};
  auto vkDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
  auto instancesBuffer = Ref<VulkanBuffer>::Create(
      "TLAS Instances", instance.data(),
      instance.size() * sizeof(VkAccelerationStructureInstanceKHR),
      VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
          VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,false);
  VkBufferDeviceAddressInfo bufferInfo{
      VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr,
      instancesBuffer->operator VkBuffer()};
  VkDeviceAddress instBufferAddr =
      vkGetBufferDeviceAddress(vkDevice, &bufferInfo);
  // Make sure the copy of the instance buffer are copied before triggering the
  // acceleration structure build
  VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
  vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                       0, 1, &barrier, 0, nullptr, 0, nullptr);

  instancesVk.data.deviceAddress = instBufferAddr;

  // Put the above into a VkAccelerationStructureGeometryKHR. We need to put the
  // instances struct in a union and label it as instance data.
  VkAccelerationStructureGeometryKHR topASGeometry{
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
  topASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
  topASGeometry.geometry.instances = instancesVk;

  // Find sizes
  VkAccelerationStructureBuildGeometryInfoKHR buildInfo{
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
  buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR |
                    VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DATA_ACCESS_KHR;
  buildInfo.geometryCount = 1;
  buildInfo.pGeometries = &topASGeometry;
  buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
  buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
  buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
  uint32_t countInstance = instance.size();

  VkAccelerationStructureBuildSizesInfoKHR sizeInfo{
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
  vkGetAccelerationStructureBuildSizesKHR(
      vkDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
      &countInstance, &sizeInfo);

#ifdef VK_NV_ray_tracing_motion_blur
  VkAccelerationStructureMotionInfoNV motionInfo{
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV};
  // motionInfo.maxInstances = countInstance;
#endif

  // Create TLAS
  if (true) {
    VkAccelerationStructureCreateInfoKHR createInfo{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    createInfo.size = sizeInfo.accelerationStructureSize;
#ifdef VK_NV_ray_tracing_motion_blur
    if (false) {
      createInfo.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_MOTION_BIT_NV;
      createInfo.pNext = &motionInfo;
    }
#endif

    // Allocating the buffer to hold the acceleration structure
    m_structureMemory = Ref<VulkanBuffer>::Create(
        "BLAS Scratch Memory", createInfo.size,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);

    createInfo.buffer = m_structureMemory->operator VkBuffer();
    vkCreateAccelerationStructureKHR(vkDevice, &createInfo, nullptr,
                                     &buildInfo.dstAccelerationStructure);
  }

  // Allocate the scratch memory
  auto scratchBuffer =
      Ref<VulkanBuffer>::Create("TLAS Scratch", sizeInfo.buildScratchSize,
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);

  VkBufferDeviceAddressInfo scratch_bufferInfo{
      VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr,
      scratchBuffer->operator VkBuffer()};
  VkDeviceAddress scratchAddress =
      vkGetBufferDeviceAddress(vkDevice, &scratch_bufferInfo);

  // Update build information
  buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
  buildInfo.scratchData.deviceAddress = scratchAddress;

  // Build Offsets info: n instances
  VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo{countInstance, 0, 0,
                                                           0};
  const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo =
      &buildOffsetInfo;

  // Build the TLAS
  vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &pBuildOffsetInfo);

  m_descriptorInfo.accelerationStructureCount = 1;
  m_descriptorInfo.pAccelerationStructures = &buildInfo.dstAccelerationStructure;
}
}  // namespace base_engine
