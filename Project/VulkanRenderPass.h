// @VulkanRenderPass.h
// @brief
// @author ICE
// @date 2023/11/24
//
// @details

#pragma once
#include "DescriptorSetManager.h"
#include "RenderPass.h"

namespace base_engine {
class VulkanRenderPass : public RenderPass {
 public:
  VulkanRenderPass(const RenderPassSpecification& spec);
  virtual ~VulkanRenderPass();
  RenderPassSpecification& GetSpecification() override {
    return specification_;
  }
  const RenderPassSpecification& GetSpecification() const override {
    return specification_;
  }
  void SetInput(std::string_view name,
                Ref<UniformBufferSet> uniformBufferSet) override;
  void SetInput(std::string_view name,
                Ref<UniformBuffer> uniformBuffer) override;
  void SetInput(std::string_view name, Ref<RendererTexture2D> texture) override;
  void SetInput(std::string_view name, Ref<Image2D> image) override;
  Ref<Image2D> GetOutput(uint32_t index) override;
  Ref<Image2D> GetDepthOutput() override;
  uint32_t GetFirstSetIndex() const override;
  Ref<FrameBuffer> GetTargetFrameBuffer() const override;
  Ref<Pipeline> GetPipeline() const override;
  bool Validate() override;
  void Bake() override;
  bool Baked() const override;
  void Prepare() override;

  bool IsInputValid(std::string_view name) const;
  const RenderPassInputDeclaration* GetInputDeclaration(
      std::string_view name) const;
  bool HasDescriptorSets() const;
  const std::vector<VkDescriptorSet>& GetDescriptorSets(
      uint32_t frameIndex) const;

 private:
  bool IsInvalidated(uint32_t set, uint32_t binding) const;
  RenderPassSpecification specification_;
  DescriptorSetManager descriptor_set_manager_;
};
}  // namespace base_engine
