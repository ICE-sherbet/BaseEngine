// @VulkanRenderPipeline.h
// @brief
// @author ICE
// @date 2023/11/24
//
// @details

#pragma once
#include <vulkan/vulkan_core.h>

#include "Pipeline.h"
#include "Ref.h"

namespace base_engine {
class VulkanRenderPipeline : public Pipeline {
 public:
  VulkanRenderPipeline(const PipelineSpecification& spec);
  virtual ~VulkanRenderPipeline();

  virtual PipelineSpecification& GetSpecification() { return specification_; }
  virtual const PipelineSpecification& GetSpecification() const {
    return specification_;
  }

  virtual void Invalidate() override;

  virtual Ref<Shader> GetShader() const override {
    return specification_.Shader;
  }

  bool IsDynamicLineWidth() const;

  VkPipeline GetVulkanPipeline() { return vulkan_pipeline_; }
  VkPipelineLayout GetVulkanPipelineLayout() { return pipeline_layout_; }

 private:
  PipelineSpecification specification_;

  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  VkPipeline vulkan_pipeline_ = VK_NULL_HANDLE;
  VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
};
}  // namespace base_engine
