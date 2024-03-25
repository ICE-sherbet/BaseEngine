#include "Raytracer.h"

#include "Application.h"
#include "AssetManager.h"
#include "PrimitiveMeshFactory.h"
#include "RaytraceMaterial.h"
#include "RendererApi.h"
#include "VulkanContext.h"
#include "VulkanIndexBuffer.h"
#include "VulkanRaytraceRenderPipeline.h"
#include "VulkanShaderCompiler.h"
#include "VulkanVertexBuffer.h"

#define RAYGUN_RAYTRACER_BINDING_ACCELERATION_STRUCTURE 0
#define RAYGUN_RAYTRACER_BINDING_OUTPUT_IMAGE 1
#define RAYGUN_RAYTRACER_BINDING_UNIFORM_BUFFER 2
#define RAYGUN_RAYTRACER_BINDING_VERTEX_BUFFER 3
#define RAYGUN_RAYTRACER_BINDING_INDEX_BUFFER 4
#define RAYGUN_RAYTRACER_BINDING_MATERIAL_BUFFER 5
#define RAYGUN_RAYTRACER_BINDING_INSTANCE_OFFSET_TABLE 6
#define RAYGUN_RAYTRACER_BINDING_ROUGH_IMAGE 7
#define RAYGUN_RAYTRACER_BINDING_NORMAL_IMAGE 8

namespace base_engine {

inline VkTransformMatrixKHR toTransformMatrixKHR(glm::mat4 matrix) {
  glm::mat4 temp = glm::transpose(matrix);
  VkTransformMatrixKHR out_matrix;
  memcpy(&out_matrix, &temp, sizeof(VkTransformMatrixKHR));
  return out_matrix;
}

Raytracer::Raytracer() {
  SetupRaytracingDescriptorSet();
  SetupRaytracingImages();
  SetupRaytracingPipeline();

  device_ = VulkanContext::Get()->GetDevice();
}

void Raytracer::SetupBottomLevelAS(const std::vector<Ref<Mesh>>& meshes) {
  Raytracer* instance = this;

  Renderer::Submit([instance, meshes]() mutable {
    auto cmd = instance->device_->GetComputeCommandBuffer(true);
    auto vk_device = instance->device_->GetVulkanDevice();
    constexpr VkFenceCreateInfo fence_create_info = {};
    VkFence fence;
    vkCreateFence(vk_device, &fence_create_info, nullptr, &fence);

    for (auto& mesh : meshes) {
      auto source = mesh->GetMeshSource();
      if (!instance->bottom_level_map_.contains(source->handle_)) {
        instance->bottom_level_map_[source->handle_] =
            Ref<BottomLevelAS>::Create(cmd, source);
      }
      ObjDesc objDesc;
      objDesc.vertexAddress =
          GetBufferDeviceAddress(vk_device, source->GetVertexBuffer()
                                                .As<VulkanVertexBuffer>()
                                                ->GetVulkanBuffer());
      objDesc.indexAddress = GetBufferDeviceAddress(
          vk_device,
          source->GetIndexBuffer().As<VulkanIndexBuffer>()->GetVulkanBuffer());
    }

    vkEndCommandBuffer(cmd);

    const auto queue = instance->device_->GetGraphicsQueue();
    VkSubmitInfo compute_submit_info{};
    compute_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    compute_submit_info.commandBufferCount = 1;
    compute_submit_info.pCommandBuffers = &cmd;

    vkQueueSubmit(queue, 1, &compute_submit_info, fence);
    vkWaitForFences(vk_device, 1, &fence, VK_TRUE, UINT64_MAX);
  });
}

void Raytracer::SetupTopLevelAS(std::vector<ObjInstance>& instances) {
  Raytracer* instance = this;
  Renderer::Submit([instance, instances]() mutable {
    std::vector<VkAccelerationStructureInstanceKHR> tlas;
    auto cmd = instance->device_->GetComputeCommandBuffer(true);
    const auto vk_device = instance->device_->GetVulkanDevice();
    constexpr VkFenceCreateInfo fence_create_info = {};
    VkFence fence;
    vkCreateFence(vk_device, &fence_create_info, nullptr, &fence);

    auto nbObj = static_cast<uint32_t>(instances.size());
    tlas.reserve(instances.size());
    for (uint32_t i = 0; i < nbObj; i++) {
      VkAccelerationStructureInstanceKHR rayInst{};
      rayInst.transform = instances[i].matrix;
      rayInst.instanceCustomIndex =
          instances[i].instanceId;  // gl_InstanceCustomIndexEXT
      rayInst.accelerationStructureReference =
          instance->bottom_level_map_[instances[i].mesh];
      rayInst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
      rayInst.mask = 0xFF;
      rayInst.instanceShaderBindingTableRecordOffset = 0;
      tlas.push_back(rayInst);
    }

    instance->top_level_as_ = Ref<TopLevelAS>::Create(cmd, tlas);

    vkEndCommandBuffer(cmd);

    const auto queue = instance->device_->GetGraphicsQueue();
    VkSubmitInfo compute_submit_info{};
    compute_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    compute_submit_info.commandBufferCount = 1;
    compute_submit_info.pCommandBuffers = &cmd;

    vkQueueSubmit(queue, 1, &compute_submit_info, fence);
    vkWaitForFences(vk_device, 1, &fence, VK_TRUE, UINT64_MAX);
  });
}

void Raytracer::UpdateRenderTarget() {
  m_descriptorSet.bind(RAYGUN_RAYTRACER_BINDING_ACCELERATION_STRUCTURE,
                       top_level_as_);

  // Bind images
  m_descriptorSet.bind(RAYGUN_RAYTRACER_BINDING_OUTPUT_IMAGE,
                       m_baseImage.As<VulkanImage2D>());
  m_descriptorSet.bind(RAYGUN_RAYTRACER_BINDING_ROUGH_IMAGE,
                       m_roughImage.As<VulkanImage2D>());
  m_descriptorSet.bind(RAYGUN_RAYTRACER_BINDING_NORMAL_IMAGE,
                       m_normalImage.As<VulkanImage2D>());

  // Bind buffers
  // m_descriptorSet.bind(RAYGUN_RAYTRACER_BINDING_UNIFORM_BUFFER,
  // uniformBuffer);
  // m_descriptorSet.bind(RAYGUN_RAYTRACER_BINDING_VERTEX_BUFFER, vertexBuffer);
  // m_descriptorSet.bind(RAYGUN_RAYTRACER_BINDING_INDEX_BUFFER, indexBuffer);
  // m_descriptorSet.bind(RAYGUN_RAYTRACER_BINDING_MATERIAL_BUFFER,materialBuffer);
  m_descriptorSet.bind(RAYGUN_RAYTRACER_BINDING_INSTANCE_OFFSET_TABLE,
                       top_level_as_->instanceOffsetTable());

  m_descriptorSet.update();
}

void Raytracer::SetupRaytracingDescriptorSet() {
  m_descriptorSet.addBinding(
      RAYGUN_RAYTRACER_BINDING_ACCELERATION_STRUCTURE, 1,
      VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
      VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

  m_descriptorSet.addBinding(RAYGUN_RAYTRACER_BINDING_OUTPUT_IMAGE, 1,
                             VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                             VK_SHADER_STAGE_RAYGEN_BIT_KHR);
  m_descriptorSet.addBinding(RAYGUN_RAYTRACER_BINDING_ROUGH_IMAGE, 1,
                             VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                             VK_SHADER_STAGE_RAYGEN_BIT_KHR);
  m_descriptorSet.addBinding(RAYGUN_RAYTRACER_BINDING_NORMAL_IMAGE, 1,
                             VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                             VK_SHADER_STAGE_RAYGEN_BIT_KHR);

  m_descriptorSet.addBinding(RAYGUN_RAYTRACER_BINDING_UNIFORM_BUFFER, 1,
                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                             VK_SHADER_STAGE_RAYGEN_BIT_KHR |
                                 VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                                 VK_SHADER_STAGE_MISS_BIT_KHR);
  m_descriptorSet.addBinding(RAYGUN_RAYTRACER_BINDING_VERTEX_BUFFER, 1,
                             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
  m_descriptorSet.addBinding(RAYGUN_RAYTRACER_BINDING_INDEX_BUFFER, 1,
                             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
  m_descriptorSet.addBinding(RAYGUN_RAYTRACER_BINDING_MATERIAL_BUFFER, 1,
                             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

  m_descriptorSet.addBinding(RAYGUN_RAYTRACER_BINDING_INSTANCE_OFFSET_TABLE, 1,
                             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

  m_descriptorSet.generate();
}

void Raytracer::SetupRaytracingImages() {
  const auto& window = Application::Get().GetWindow();
  ImageSpecification spec;
  spec.Format = ImageFormat::RGBA16F;
  spec.Usage = ImageUsage::Attachment;
  spec.Transfer = false;
  spec.Width = window.GetWidth();
  spec.Height = window.GetHeight();
  spec.DebugName = "RT Base Image";
  m_baseImage = Image2D::Create(spec);

  spec.DebugName = "RT Normal Image";
  m_normalImage = Image2D::Create(spec);

  spec.DebugName = "RT Rough Image";
  m_roughImage = Image2D::Create(spec);

  spec.DebugName = "RT Final Image";
  m_finalImage = Image2D::Create(spec);

  spec.DebugName = "RT Rough Color A";
  m_roughColorsA = Image2D::Create(spec);

  spec.DebugName = "RT Rough Color B";
  m_roughColorsA = Image2D::Create(spec);

  spec.DebugName = "RT Rough Transition";
  spec.Format = ImageFormat::RED8SN;
  m_roughTransitions = Image2D::Create(spec);
}

void Raytracer::SetupRaytracingPipeline() {
  auto spec = PipelineSpecification{};
  pipeline_ = new VulkanRaytracePipeline{spec};
  pipeline_->CreatePipeline(m_descriptorSet.layout());
}
}  // namespace base_engine
