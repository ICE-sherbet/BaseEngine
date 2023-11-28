#include "VulkanRenderPass.h"

namespace base_engine
{
VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec) : specification_(spec)
{
  DescriptorSetManagerSpecification dmSpec;
  dmSpec.DebugName = spec.DebugName;
  dmSpec.Shader = spec.Pipeline->GetSpecification().Shader.As<VulkanShader>();
  dmSpec.StartSet = 1;
  descriptor_set_manager_ = DescriptorSetManager(dmSpec);
}

VulkanRenderPass::~VulkanRenderPass() {}

bool VulkanRenderPass::IsInvalidated(uint32_t set, uint32_t binding) const {
  return descriptor_set_manager_.IsInvalidated(set, binding);
}

void VulkanRenderPass::SetInput(std::string_view name,
                                Ref<UniformBufferSet> uniformBufferSet) {
  descriptor_set_manager_.SetInput(name, uniformBufferSet);
}

void VulkanRenderPass::SetInput(std::string_view name,
                                Ref<UniformBuffer> uniformBuffer) {
  descriptor_set_manager_.SetInput(name, uniformBuffer);
}

void VulkanRenderPass::SetInput(std::string_view name, Ref<RendererTexture2D> texture)
{
	descriptor_set_manager_.SetInput(name, texture);
}

void VulkanRenderPass::SetInput(std::string_view name, Ref<Image2D> image) {
  descriptor_set_manager_.SetInput(name, image);
}

Ref<Image2D> VulkanRenderPass::GetOutput(uint32_t index) {
  Ref<FrameBuffer> framebuffer =
      specification_.Pipeline->GetSpecification().TargetFrameBuffer;
  if (index > framebuffer->GetColorAttachmentCount() + 1)
    return nullptr;  // Invalid index
  if (index < framebuffer->GetColorAttachmentCount())
    return framebuffer->GetImage(index);
  return framebuffer->GetDepthImage();
}
Ref<Image2D> VulkanRenderPass::GetDepthOutput() {
  Ref<FrameBuffer> framebuffer =
      specification_.Pipeline->GetSpecification().TargetFrameBuffer;
  if (!framebuffer->HasDepthAttachment()) return nullptr;  // No depth output
  return framebuffer->GetDepthImage();
}
uint32_t VulkanRenderPass::GetFirstSetIndex() const {
  return descriptor_set_manager_.GetFirstSetIndex();
}
Ref<FrameBuffer> VulkanRenderPass::GetTargetFrameBuffer() const {
  return specification_.Pipeline->GetSpecification().TargetFrameBuffer;
}
Ref<Pipeline> VulkanRenderPass::GetPipeline() const {
  return specification_.Pipeline;
}
bool VulkanRenderPass::Validate() { return descriptor_set_manager_.Validate(); }
void VulkanRenderPass::Bake() { descriptor_set_manager_.Bake(); }

bool VulkanRenderPass::Baked() const
{
  return (bool)descriptor_set_manager_.GetDescriptorPool();
}

void VulkanRenderPass::Prepare() {
  descriptor_set_manager_.InvalidateAndUpdate();
}
bool VulkanRenderPass::HasDescriptorSets() const {
  return descriptor_set_manager_.HasDescriptorSets();
}
const std::vector<VkDescriptorSet>& VulkanRenderPass::GetDescriptorSets(
    uint32_t frameIndex) const {
  BE_CORE_ASSERT(!descriptor_set_manager_.descriptor_sets.empty());
  if (frameIndex > 0 && descriptor_set_manager_.descriptor_sets.size() == 1)
    return descriptor_set_manager_
        .descriptor_sets[0];  // Frame index is irrelevant for this type of
                               // render pass
  return descriptor_set_manager_.descriptor_sets[frameIndex];
}
bool VulkanRenderPass::IsInputValid(std::string_view name) const {
  std::string nameStr(name);
  return descriptor_set_manager_.InputDeclarations.find(nameStr) !=
         descriptor_set_manager_.InputDeclarations.end();
}
const RenderPassInputDeclaration* VulkanRenderPass::GetInputDeclaration(
    std::string_view name) const {
  std::string nameStr(name);
  if (!descriptor_set_manager_.InputDeclarations.contains(nameStr)
  )
    return nullptr;
  const RenderPassInputDeclaration& decl =
      descriptor_set_manager_.InputDeclarations.at(nameStr);
  return &decl;
}
}
