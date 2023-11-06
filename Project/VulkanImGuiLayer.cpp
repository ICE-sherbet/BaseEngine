#include "VulkanImGuiLayer.h"

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include "Application.h"
#include "BaseEngineCore.h"
#include "Platform/Windows/Window.h"
#include "RendererApi.h"
#include "VulkanContext.h"
#include "VulkanRendererContext.h"
#include "imgui.h"

namespace base_engine {
namespace {
std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;
}

void VulkanImGuiLayer::Init() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  Renderer::Submit([]() {
    Application& app = Application::Get();
    const auto window =
        static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
    const auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

    VkDescriptorPool descriptor_pool;
    {
      constexpr VkDescriptorPoolSize pool_sizes[] = {
          {VK_DESCRIPTOR_TYPE_SAMPLER, 100},
          {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
          {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100},
          {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
          {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100},
          {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100},
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
          {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
          {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100},
          {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100}};
      VkDescriptorPoolCreateInfo pool_info = {};
      pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
      pool_info.maxSets = 100 * IM_ARRAYSIZE(pool_sizes);
      pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
      pool_info.pPoolSizes = pool_sizes;
      vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool);
    }

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};

    init_info.Instance = VulkanContext::GetVkInstance();
    init_info.PhysicalDevice =
        VulkanContext::GetCurrentDevice()->GetVulkanPhysicalDevice();
    init_info.Device = device;
    init_info.QueueFamily =
        VulkanContext::GetCurrentDevice()->GetGraphicsIndex();
    init_info.Queue = VulkanContext::GetCurrentDevice()->GetGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptor_pool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = 2;
    VulkanSwapChain& swapChain =
        static_cast<Window&>(Application::Get().GetWindow()).GetSwapChain();
    init_info.ImageCount = swapChain.GetImageCount();

    ImGui_ImplVulkan_Init(&init_info, swapChain.GetRenderPass());

    {
      const VkCommandBuffer command_buffer =
          VulkanContext::GetCurrentDevice()->GetCommandBuffer(true);
      ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
      VulkanContext::GetCurrentDevice()->FlushCommandBuffer(command_buffer);

      vkDeviceWaitIdle(device);
      ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    uint32_t frames_in_flight = 3;
    s_ImGuiCommandBuffers.resize(frames_in_flight);
    for (uint32_t i = 0; i < frames_in_flight; i++)
      s_ImGuiCommandBuffers[i] =
          VulkanContext::GetCurrentDevice()->CreateSecondaryCommandBuffer(
              "ImGuiSecondaryCoommandBuffer");
  });
}

void VulkanImGuiLayer::Begin() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!"
                                  // and append into it.
  ImGui::Text("This is some useful text.");  // Display some text (you can use
                                             // a format strings too)
  ImGui::End();
}

void VulkanImGuiLayer::End() {
  ImGui::Render();

  VulkanSwapChain& swapChain =
      static_cast<Window&>(Application::Get().GetWindow()).GetSwapChain();

  VkClearValue clearValues[2];
  clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  uint32_t width = swapChain.GetWidth();
  uint32_t height = swapChain.GetHeight();

  uint32_t commandBufferIndex = swapChain.GetCurrentBufferIndex();

  VkCommandBufferBeginInfo drawCmdBufInfo = {};
  drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  drawCmdBufInfo.pNext = nullptr;

  VkCommandBuffer drawCommandBuffer = swapChain.GetCurrentDrawCommandBuffer();
  vkBeginCommandBuffer(drawCommandBuffer, &drawCmdBufInfo);

  VkRenderPassBeginInfo renderPassBeginInfo = {};
  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.pNext = nullptr;
  renderPassBeginInfo.renderPass = swapChain.GetRenderPass();
  renderPassBeginInfo.renderArea.offset.x = 0;
  renderPassBeginInfo.renderArea.offset.y = 0;
  renderPassBeginInfo.renderArea.extent.width = width;
  renderPassBeginInfo.renderArea.extent.height = height;
  renderPassBeginInfo.clearValueCount = 2;  // Color + depth
  renderPassBeginInfo.pClearValues = clearValues;
  renderPassBeginInfo.framebuffer = swapChain.GetCurrentFramebuffer();

  vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo,
                       VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

  VkCommandBufferInheritanceInfo inheritanceInfo = {};
  inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
  inheritanceInfo.renderPass = swapChain.GetRenderPass();
  inheritanceInfo.framebuffer = swapChain.GetCurrentFramebuffer();

  VkCommandBufferBeginInfo cmdBufInfo = {};
  cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
  cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

  vkBeginCommandBuffer(s_ImGuiCommandBuffers[commandBufferIndex], &cmdBufInfo);

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = (float)height;
  viewport.height = -(float)height;
  viewport.width = (float)width;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(s_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &viewport);

  VkRect2D scissor = {};
  scissor.extent.width = width;
  scissor.extent.height = height;
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  vkCmdSetScissor(s_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &scissor);

  ImDrawData* main_draw_data = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(main_draw_data,
                                  s_ImGuiCommandBuffers[commandBufferIndex]);

  vkEndCommandBuffer(s_ImGuiCommandBuffers[commandBufferIndex]);

  std::vector<VkCommandBuffer> commandBuffers;
  commandBuffers.push_back(s_ImGuiCommandBuffers[commandBufferIndex]);

  vkCmdExecuteCommands(drawCommandBuffer, uint32_t(commandBuffers.size()),
                       commandBuffers.data());

  vkCmdEndRenderPass(drawCommandBuffer);

  vkEndCommandBuffer(drawCommandBuffer);

  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  // Update and Render additional Platform Windows
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
}
}  // namespace base_engine
