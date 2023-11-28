#include "VulkanRenderer.h"

#include <glm/gtc/type_ptr.hpp>

#include "Application.h"
#include "Platform/Windows/Window.h"
#include "VulkanContext.h"
#include "VulkanFrameBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanMaterial.h"
#include "VulkanRenderCommandBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderPipeline.h"
#include "VulkanVertexBuffer.h"

namespace base_engine {
void VulkanRenderer::Init() {}

void VulkanRenderer::Shutdown() {}

void VulkanRenderer::BeginFrame() {
  Renderer::Submit([]() {
    VulkanSwapChain& swap_chain =
        dynamic_cast<Window&>(Application::Get().GetWindow()).GetSwapChain();
  });
}

void VulkanRenderer::EndFrame() {}

static PFN_vkCmdBeginDebugUtilsLabelEXT fpCmdBeginDebugUtilsLabelEXT = nullptr;
static PFN_vkCmdEndDebugUtilsLabelEXT fpCmdEndDebugUtilsLabelEXT = nullptr;

void VulkanRenderer::BeginRenderPass(
    Ref<RenderCommandBuffer> renderCommandBuffer, Ref<RenderPass> renderPass,
    bool explicitClear) {
  Renderer::Submit([renderCommandBuffer, renderPass, explicitClear]() {
    uint32_t frameIndex = Renderer::RT_GetCurrentFrameIndex();
    VkCommandBuffer commandBuffer =
        renderCommandBuffer.As<VulkanRenderCommandBuffer>()
            ->GetActiveCommandBuffer();

    VkDebugUtilsLabelEXT debugLabel{};
    debugLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    memcpy(&debugLabel.color,
           glm::value_ptr(renderPass->GetSpecification().MarkerColor),
           sizeof(float) * 4);
    debugLabel.pLabelName = renderPass->GetSpecification().DebugName.c_str();
    if (fpCmdBeginDebugUtilsLabelEXT == nullptr) {
      fpCmdBeginDebugUtilsLabelEXT =
          reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
              vkGetInstanceProcAddr(VulkanContext::Get()->GetVkInstance(),
                                    "vkCmdBeginDebugUtilsLabelEXT"));
    }
    fpCmdBeginDebugUtilsLabelEXT(commandBuffer, &debugLabel);

    auto fb = renderPass->GetSpecification()
                  .Pipeline->GetSpecification()
                  .TargetFrameBuffer;
    Ref<VulkanFrameBuffer> framebuffer = fb.As<VulkanFrameBuffer>();
    const auto& fbSpec = framebuffer->GetSpecification();

    uint32_t width = framebuffer->GetWidth();
    uint32_t height = framebuffer->GetHeight();

    VkViewport viewport = {};
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = framebuffer->GetRenderPass();
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    if (framebuffer->GetSpecification().SwapChainTarget) {
      VulkanSwapChain& swapChain =
          static_cast<Window&>(Application::Get().GetWindow()).GetSwapChain();
      width = swapChain.GetWidth();
      height = swapChain.GetHeight();
      renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassBeginInfo.pNext = nullptr;
      renderPassBeginInfo.renderPass = framebuffer->GetRenderPass();
      renderPassBeginInfo.renderArea.offset.x = 0;
      renderPassBeginInfo.renderArea.offset.y = 0;
      renderPassBeginInfo.renderArea.extent.width = width;
      renderPassBeginInfo.renderArea.extent.height = height;
      renderPassBeginInfo.framebuffer = swapChain.GetCurrentFramebuffer();

      viewport.x = 0.0f;
      viewport.y = (float)height;
      viewport.width = (float)width;
      viewport.height = -(float)height;
    } else {
      width = framebuffer->GetWidth();
      height = framebuffer->GetHeight();
      renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassBeginInfo.pNext = nullptr;
      renderPassBeginInfo.renderPass = framebuffer->GetRenderPass();
      renderPassBeginInfo.renderArea.offset.x = 0;
      renderPassBeginInfo.renderArea.offset.y = 0;
      renderPassBeginInfo.renderArea.extent.width = width;
      renderPassBeginInfo.renderArea.extent.height = height;
      renderPassBeginInfo.framebuffer = framebuffer->GetVulkanFramebuffer();

      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = (float)width;
      viewport.height = (float)height;
    }

    // TODO: Does our framebuffer have a depth attachment?
    const auto& clearValues = framebuffer->GetVulkanClearValues();
    renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    // Update dynamic viewport state
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    // Update dynamic scissor state
    VkRect2D scissor = {};
    scissor.extent.width = width;
    scissor.extent.height = height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // TODO: automatic layout transitions for input resources

    // Bind Vulkan Pipeline
    Ref<VulkanRenderPipeline> vulkanPipeline =
        renderPass->GetSpecification().Pipeline.As<VulkanRenderPipeline>();
    VkPipeline vPipeline = vulkanPipeline->GetVulkanPipeline();
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      vPipeline);

    if (vulkanPipeline->IsDynamicLineWidth())
      vkCmdSetLineWidth(commandBuffer,
                        vulkanPipeline->GetSpecification().LineWidth);

    // Bind input descriptors (starting from set 1, set 0 is for per-draw)
    Ref<VulkanRenderPass> vulkanRenderPass = renderPass.As<VulkanRenderPass>();
    vulkanRenderPass->Prepare();
    if (vulkanRenderPass->HasDescriptorSets()) {
      const auto& descriptorSets =
          vulkanRenderPass->GetDescriptorSets(frameIndex);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              vulkanPipeline->GetVulkanPipelineLayout(),
                              vulkanRenderPass->GetFirstSetIndex(),
                              (uint32_t)descriptorSets.size(),
                              descriptorSets.data(), 0, nullptr);
    }
  });
}

void VulkanRenderer::EndRenderPass(
    Ref<RenderCommandBuffer> renderCommandBuffer) {
  Renderer::Submit([renderCommandBuffer]() {
    uint32_t frameIndex = Renderer::RT_GetCurrentFrameIndex();
    VkCommandBuffer commandBuffer =
        renderCommandBuffer.As<VulkanRenderCommandBuffer>()
            ->GetActiveCommandBuffer();

    vkCmdEndRenderPass(commandBuffer);
    if (fpCmdEndDebugUtilsLabelEXT == nullptr) {
      fpCmdEndDebugUtilsLabelEXT =
          reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
              vkGetInstanceProcAddr(VulkanContext::Get()->GetVkInstance(),
                                    "vkCmdEndDebugUtilsLabelEXT"));
    }
    fpCmdEndDebugUtilsLabelEXT(commandBuffer);
  });
}

void VulkanRenderer::RenderGeometry(
    Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline,
    Ref<Material> material, Ref<VertexBuffer> vertexBuffer,
    Ref<IndexBuffer> indexBuffer, const glm::mat4& transform,
    uint32_t indexCount) {
  Ref<VulkanMaterial> vulkanMaterial = material.As<VulkanMaterial>();
  if (indexCount == 0) indexCount = indexBuffer->GetCount();

  Renderer::Submit([renderCommandBuffer, pipeline, vulkanMaterial, vertexBuffer,
                    indexBuffer, transform, indexCount]() mutable {
    uint32_t frameIndex = Renderer::RT_GetCurrentFrameIndex();
    VkCommandBuffer commandBuffer =
        renderCommandBuffer.As<VulkanRenderCommandBuffer>()
            ->GetActiveCommandBuffer();

    Ref<VulkanRenderPipeline> vulkanPipeline =
        pipeline.As<VulkanRenderPipeline>();

    VkPipelineLayout layout = vulkanPipeline->GetVulkanPipelineLayout();

    auto vulkanMeshVB = vertexBuffer.As<VulkanVertexBuffer>();
    VkBuffer vbMeshBuffer = vulkanMeshVB->GetVulkanBuffer();
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vbMeshBuffer, offsets);

    auto vulkanMeshIB = indexBuffer.As<VulkanIndexBuffer>();
    VkBuffer ibBuffer = vulkanMeshIB->GetVulkanBuffer();
    vkCmdBindIndexBuffer(commandBuffer, ibBuffer, 0, VK_INDEX_TYPE_UINT32);

    VkDescriptorSet descriptorSet =
        vulkanMaterial->GetDescriptorSet(frameIndex);
    if (descriptorSet)
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              layout, 0, 1, &descriptorSet, 0, nullptr);

    vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(glm::mat4), &transform);
    Buffer uniformStorageBuffer = vulkanMaterial->GetUniformStorageBuffer();
    if (uniformStorageBuffer)
      vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT,
                         sizeof(glm::mat4), uniformStorageBuffer.Size,
                         uniformStorageBuffer.Data);

    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  });
}
}  // namespace base_engine
