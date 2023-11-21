#include "VulkanSwapChain.h"

#include "Application.h"

namespace base_engine {
VulkanSwapChain* VulkanSwapChain::instance_ = nullptr;

void VulkanSwapChain::Init(VkInstance instance,
                           const Ref<VulkanDevice>& device) {
  instance_ = this; 
  m_Instance = instance;
  m_Device = device;
}

void VulkanSwapChain::InitSurface(GLFWwindow* window_handle) {
  const VkPhysicalDevice physical_device = m_Device->GetVulkanPhysicalDevice();
  auto r =
      glfwCreateWindowSurface(m_Instance, window_handle, nullptr, &m_Surface);

  SelectSurfaceFormat(VK_FORMAT_B8G8R8A8_UNORM);

  uint32_t queue_count;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, NULL);
  BE_CORE_ASSERT(queue_count >= 1);

  std::vector<VkQueueFamilyProperties> queue_props(queue_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count,
                                           queue_props.data());

  std::vector<VkBool32> supports_present(queue_count);

  for (uint32_t i = 0; i < queue_count; i++) {
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, m_Surface,
                                         &supports_present[i]);
  }
  uint32_t graphics_queue_node_index = UINT32_MAX;
  uint32_t present_queue_node_index = UINT32_MAX;
  for (uint32_t i = 0; i < queue_count; i++) {
    if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      if (graphics_queue_node_index == UINT32_MAX) {
        graphics_queue_node_index = i;
      }

      if (supports_present[i] == VK_TRUE) {
        graphics_queue_node_index = i;
        present_queue_node_index = i;
        break;
      }
    }
  }
  if (present_queue_node_index == UINT32_MAX) {
    for (uint32_t i = 0; i < queue_count; ++i) {
      if (supports_present[i] == VK_TRUE) {
        present_queue_node_index = i;
        break;
      }
    }
  }
  BE_CORE_ASSERT(graphics_queue_node_index != UINT32_MAX);
  BE_CORE_ASSERT(present_queue_node_index != UINT32_MAX);
  m_QueueNodeIndex = graphics_queue_node_index;
}

void VulkanSwapChain::Create(GLFWwindow* window_handle, bool vsync) {
  const VkPhysicalDevice physical_device = m_Device->GetVulkanPhysicalDevice();
  VkSurfaceCapabilitiesKHR surf_caps;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, m_Surface,
                                            &surf_caps);

  auto extent = surf_caps.currentExtent;
  if (extent.width == ~0u) {
    int width, height;
    glfwGetWindowSize(window_handle, &width, &height);
    extent.width = static_cast<uint32_t>(width);
    extent.height = static_cast<uint32_t>(height);
  }

  Create(extent.width, extent.height, vsync);
}

void VulkanSwapChain::Create(uint32_t width, uint32_t height, bool vsync) {
  m_Width = width;
  m_Height = height;

  const VkPhysicalDevice physical_device = m_Device->GetVulkanPhysicalDevice();
  VkSurfaceCapabilitiesKHR surf_caps;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, m_Surface,
                                            &surf_caps);

  auto imageCount = (std::max)(2u, surf_caps.minImageCount);
  auto extent = surf_caps.currentExtent;

  VkSwapchainCreateInfoKHR ci{};
  ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  ci.surface = m_Surface;
  ci.minImageCount = imageCount;
  ci.imageFormat = m_ColorFormat;
  ci.imageColorSpace = m_ColorSpace;
  ci.imageExtent = extent;
  ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  ci.preTransform = surf_caps.currentTransform;
  ci.imageArrayLayers = 1;
  ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ci.queueFamilyIndexCount = 0;
  ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  ci.oldSwapchain = VK_NULL_HANDLE;
  ci.clipped = VK_TRUE;
  ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  auto device = m_Device->GetVulkanDevice();

  auto result = vkCreateSwapchainKHR(device, &ci, nullptr, &m_SwapChain);

  m_extent = extent;

  for (const auto& [Image, ImageView] : m_Images)
    vkDestroyImageView(device, ImageView, nullptr);
  m_Images.clear();

  vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, NULL);
  m_Images.resize(m_ImageCount);
  m_VulkanImages.resize(m_ImageCount);
  vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount,
                          m_VulkanImages.data());

  m_Images.resize(m_ImageCount);
  for (uint32_t i = 0; i < m_ImageCount; i++) {
    VkImageViewCreateInfo colorAttachmentView = {};
    colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    colorAttachmentView.pNext = NULL;
    colorAttachmentView.format = m_ColorFormat;
    colorAttachmentView.image = m_VulkanImages[i];
    colorAttachmentView.components = {
        VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A};
    colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorAttachmentView.subresourceRange.baseMipLevel = 0;
    colorAttachmentView.subresourceRange.levelCount = 1;
    colorAttachmentView.subresourceRange.baseArrayLayer = 0;
    colorAttachmentView.subresourceRange.layerCount = 1;
    colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorAttachmentView.flags = 0;

    m_Images[i].Image = m_VulkanImages[i];

    vkCreateImageView(device, &colorAttachmentView, nullptr,
                      &m_Images[i].ImageView);
  }

  // Create command buffers
  {
    for (auto& commandBuffer : m_CommandBuffers)
      vkDestroyCommandPool(device, commandBuffer.CommandPool, nullptr);

    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = m_QueueNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    m_CommandBuffers.resize(m_ImageCount);
    for (auto& commandBuffer : m_CommandBuffers) {
      vkCreateCommandPool(device, &cmdPoolInfo, nullptr,
                          &commandBuffer.CommandPool);

      commandBufferAllocateInfo.commandPool = commandBuffer.CommandPool;
      vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
                               &commandBuffer.CommandBuffer);
    }
  }
  if (!m_Semaphores.RenderComplete || !m_Semaphores.PresentComplete) {
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo,
                      nullptr, &m_Semaphores.RenderComplete);
    vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo,
                      nullptr, &m_Semaphores.PresentComplete);
  }

  if (m_WaitFences.size() != m_ImageCount) {
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    m_WaitFences.resize(m_ImageCount);
    for (auto& fence : m_WaitFences) {
      vkCreateFence(m_Device->GetVulkanDevice(), &fenceCreateInfo, nullptr,
                    &fence);
    }
  }

  VkPipelineStageFlags pipelineStageFlags =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  m_SubmitInfo = {};
  m_SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  m_SubmitInfo.pWaitDstStageMask = &pipelineStageFlags;
  m_SubmitInfo.waitSemaphoreCount = 1;
  m_SubmitInfo.pWaitSemaphores = &m_Semaphores.PresentComplete;
  m_SubmitInfo.signalSemaphoreCount = 1;
  m_SubmitInfo.pSignalSemaphores = &m_Semaphores.RenderComplete;

  // Render Pass
  VkAttachmentDescription colorAttachmentDesc = {};
  // Color attachment
  colorAttachmentDesc.format = m_ColorFormat;
  colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorReference = {};
  colorReference.attachment = 0;
  colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthReference = {};
  depthReference.attachment = 1;
  depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpassDescription = {};
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorReference;
  subpassDescription.inputAttachmentCount = 0;
  subpassDescription.pInputAttachments = nullptr;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments = nullptr;
  subpassDescription.pResolveAttachments = nullptr;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachmentDesc;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpassDescription;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  vkCreateRenderPass(m_Device->GetVulkanDevice(), &renderPassInfo, nullptr,
                     &m_RenderPass);

  {
    for (auto& frame_buffer : m_Framebuffers)
      vkDestroyFramebuffer(device, frame_buffer, nullptr);

    VkFramebufferCreateInfo frame_buffer_create_info = {};
    frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_create_info.renderPass = m_RenderPass;
    frame_buffer_create_info.attachmentCount = 1;
    frame_buffer_create_info.width = m_Width;
    frame_buffer_create_info.height = m_Height;
    frame_buffer_create_info.layers = 1;

    m_Framebuffers.resize(m_ImageCount);
    for (uint32_t i = 0; i < m_Framebuffers.size(); i++) {
      frame_buffer_create_info.pAttachments = &m_Images[i].ImageView;
      vkCreateFramebuffer(m_Device->GetVulkanDevice(),
                          &frame_buffer_create_info, nullptr,
                          &m_Framebuffers[i]);
    }
  }
}

void VulkanSwapChain::Destroy()
{
  instance_ = nullptr;
}

void VulkanSwapChain::OnResize(uint32_t width, uint32_t height) {
  auto device = m_Device->GetVulkanDevice();
  vkDeviceWaitIdle(device);
  Create(width, height, m_VSync);
  vkDeviceWaitIdle(device);
}

void VulkanSwapChain::BeginFrame() { m_CurrentImageIndex = AcquireNextImage(); }

void VulkanSwapChain::Present() {
  constexpr VkPipelineStageFlags waitStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pWaitDstStageMask = &waitStageMask;
  submitInfo.pWaitSemaphores = &m_Semaphores.PresentComplete;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &m_Semaphores.RenderComplete;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pCommandBuffers =
      &m_CommandBuffers[m_CurrentBufferIndex].CommandBuffer;
  submitInfo.commandBufferCount = 1;

  vkResetFences(m_Device->GetVulkanDevice(), 1,
                &m_WaitFences[m_CurrentBufferIndex]);
  vkQueueSubmit(m_Device->GetGraphicsQueue(), 1, &submitInfo,
                m_WaitFences[m_CurrentBufferIndex]);

  VkResult result;
  {
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_SwapChain;
    presentInfo.pImageIndices = &m_CurrentImageIndex;

    presentInfo.pWaitSemaphores = &m_Semaphores.RenderComplete;
    presentInfo.waitSemaphoreCount = 1;
    result = vkQueuePresentKHR(m_Device->GetGraphicsQueue(), &presentInfo);
  }

  if (result != VK_SUCCESS) {
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      OnResize(m_Width, m_Height);
    } else {
    }
  }

  {
    m_CurrentBufferIndex = (m_CurrentBufferIndex + 1) % 3;
    // Make sure the frame we're requesting has finished rendering
    vkWaitForFences(m_Device->GetVulkanDevice(), 1,
                    &m_WaitFences[m_CurrentBufferIndex], VK_TRUE, UINT64_MAX);
  }
}

uint32_t VulkanSwapChain::AcquireNextImage() {
  uint32_t image_index;
  VkResult result =
      vkAcquireNextImageKHR(m_Device->GetVulkanDevice(), m_SwapChain,
                            UINT64_MAX, m_Semaphores.PresentComplete,
                            reinterpret_cast<VkFence>(nullptr), &image_index);
  if (result != VK_SUCCESS) {
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      OnResize(m_Width, m_Height);
      vkAcquireNextImageKHR(m_Device->GetVulkanDevice(), m_SwapChain,
                            UINT64_MAX, m_Semaphores.PresentComplete,
                            (VkFence) nullptr, &image_index);
    }
  }

  return image_index;
}

void VulkanSwapChain::SelectSurfaceFormat(const VkFormat select_format) {
  const VkPhysicalDevice physical_device = m_Device->GetVulkanPhysicalDevice();

  uint32_t surface_format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_Surface,
                                       &surface_format_count, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(surface_format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_Surface,
                                       &surface_format_count, formats.data());

  // 検索して一致するフォーマットを見つける.
  for (const auto& [format, colorSpace] : formats) {
    if (format == select_format) {
      m_ColorFormat = format;
      m_ColorSpace = colorSpace;
    }
  }
}
}  // namespace base_engine
