// @Raytracer.h
// @brief
// @author ICE
// @date 2024/03/12
//
// @details

#pragma once

#include "BottomLevelAS.h"
#include "DescriptorSet.h"
#include "ObjectDesc.h"
#include "Ref.h"
#include "RenderPass.h"
#include "TopLevelAS.h"
#include "VulkanRaytraceRenderPipeline.h"

namespace base_engine {
class VulkanDevice;

class Raytracer : RefCounted {
 public:
  Raytracer();

  void SetupBottomLevelAS(const std::vector<Ref<Mesh>>& meshes);
  void SetupTopLevelAS(std::vector<ObjInstance>& instances);

  void UpdateRenderTarget();
  Ref<Image2D> DoRaytracing(VkCommandBuffer cmd);

 private:
  void SetupRaytracingDescriptorSet();

  void SetupPostProcessing() {}

  void SetupRaytracingImages();

  void SetupRaytracingPipeline();

  Ref<VulkanRaytracePipeline> pipeline_;
  Ref<RenderPass> render_pass_;

  DescriptorSet m_descriptorSet;
  std::vector<ObjDesc> m_objDescs;
  Ref<VulkanBuffer> obj_desc_buffer_;

  using MeshKey = AssetHandle;
  std::map<MeshKey, Ref<BottomLevelAS>> bottom_level_map_;
  Ref<TopLevelAS> top_level_as_;

  // these are rendered to directly in ray tracing
  Ref<Image2D> m_baseImage;
  Ref<Image2D> m_normalImage;
  Ref<Image2D> m_roughImage;

  // final image storage
  Ref<Image2D> m_finalImage;

  // intermediate buffers
  Ref<Image2D> m_roughTransitions;
  Ref<Image2D> m_roughColorsA, m_roughColorsB;

  Ref<VulkanDevice> device_;
};
}  // namespace base_engine