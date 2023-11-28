#include "VulkanSwapChain.h"

#include "Application.h"
#include "RendererApi.h"

namespace base_engine {
VulkanSwapChain* VulkanSwapChain::instance_ = nullptr;

VulkanSwapChain::VulkanSwapChain()
{
}

void VulkanSwapChain::Init(VkInstance instance,
                           const Ref<VulkanDevice>& device) {
  instance_ = this; 
  vk_instance_ = instance;
  device_ = device;
}

void VulkanSwapChain::InitSurface(GLFWwindow* window_handle) {
  const VkPhysicalDevice physical_device = device_->GetVulkanPhysicalDevice();
  auto r =
      glfwCreateWindowSurface(vk_instance_, window_handle, nullptr, &surface_);

  SelectSurfaceFormat(VK_FORMAT_B8G8R8A8_UNORM);

  uint32_t queue_count;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, NULL);
  BE_CORE_ASSERT(queue_count >= 1);

  std::vector<VkQueueFamilyProperties> queue_props(queue_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count,
                                           queue_props.data());

  std::vector<VkBool32> supports_present(queue_count);

  for (uint32_t i = 0; i < queue_count; i++) {
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface_,
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
  queue_node_index_ = graphics_queue_node_index;
}

void VulkanSwapChain::Create(GLFWwindow* window_handle, bool vsync) {
  const VkPhysicalDevice physical_device = device_->GetVulkanPhysicalDevice();
  VkSurfaceCapabilitiesKHR surf_caps;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface_,
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
  width_ = width;
  height_ = height;

  const VkPhysicalDevice physical_device = device_->GetVulkanPhysicalDevice();
  VkSurfaceCapabilitiesKHR surf_caps;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface_,
                                            &surf_caps);

  auto imageCount = (std::max)(3u, surf_caps.minImageCount);
  auto extent = surf_caps.currentExtent;

  VkSwapchainCreateInfoKHR ci{};
  ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  ci.surface = surface_;
  ci.minImageCount = imageCount;
  ci.imageFormat = color_format_;
  ci.imageColorSpace = color_space_;
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

  auto device = device_->GetVulkanDevice();

  auto result = vkCreateSwapchainKHR(device, &ci, nullptr, &swap_chain_);

  extent_ = extent;

  for (const auto& [Image, ImageView] : images_)
    vkDestroyImageView(device, ImageView, nullptr);
  images_.clear();

  vkGetSwapchainImagesKHR(device, swap_chain_, &image_count_, NULL);
  images_.resize(image_count_);
  vulkan_images_.resize(image_count_);
  vkGetSwapchainImagesKHR(device, swap_chain_, &image_count_,
                          vulkan_images_.data());

  images_.resize(image_count_);
  for (uint32_t i = 0; i < image_count_; i++) {
    VkImageViewCreateInfo colorAttachmentView = {};
    colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    colorAttachmentView.pNext = NULL;
    colorAttachmentView.format = color_format_;
    colorAttachmentView.image = vulkan_images_[i];
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

    images_[i].Image = vulkan_images_[i];

    vkCreateImageView(device, &colorAttachmentView, nullptr,
                      &images_[i].ImageView);
  }

  // Create command buffers
  {
    for (auto& commandBuffer : command_buffers_)
      vkDestroyCommandPool(device, commandBuffer.CommandPool, nullptr);

    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = queue_node_index_;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    command_buffers_.resize(image_count_);
    for (auto& commandBuffer : command_buffers_) {
      vkCreateCommandPool(device, &cmdPoolInfo, nullptr,
                          &commandBuffer.CommandPool);

      commandBufferAllocateInfo.commandPool = commandBuffer.CommandPool;
      vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
                               &commandBuffer.CommandBuffer);
    }
  }
  if (!semaphores_.RenderComplete || !semaphores_.PresentComplete) {
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(device_->GetVulkanDevice(), &semaphoreCreateInfo,
                      nullptr, &semaphores_.RenderComplete);
    vkCreateSemaphore(device_->GetVulkanDevice(), &semaphoreCreateInfo,
                      nullptr, &semaphores_.PresentComplete);
  }

  if (wait_fences_.size() != image_count_) {
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    wait_fences_.resize(image_count_);
    for (auto& fence : wait_fences_) {
      vkCreateFence(device_->GetVulkanDevice(), &fenceCreateInfo, nullptr,
                    &fence);
    }
  }

  VkPipelineStageFlags pipelineStageFlags =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  submit_info_ = {};
  submit_info_.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info_.pWaitDstStageMask = &pipelineStageFlags;
  submit_info_.waitSemaphoreCount = 1;
  submit_info_.pWaitSemaphores = &semaphores_.PresentComplete;
  submit_info_.signalSemaphoreCount = 1;
  submit_info_.pSignalSemaphores = &semaphores_.RenderComplete;

  // Render Pass
  VkAttachmentDescription colorAttachmentDesc = {};
  // Color attachment
  colorAttachmentDesc.format = color_format_;
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

  vkCreateRenderPass(device_->GetVulkanDevice(), &renderPassInfo, nullptr,
                     &render_pass_);

  {
    for (auto& frame_buffer : framebuffers_)
      vkDestroyFramebuffer(device, frame_buffer, nullptr);

    VkFramebufferCreateInfo frame_buffer_create_info = {};
    frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_create_info.renderPass = render_pass_;
    frame_buffer_create_info.attachmentCount = 1;
    frame_buffer_create_info.width = width_;
    frame_buffer_create_info.height = height_;
    frame_buffer_create_info.layers = 1;

    framebuffers_.resize(image_count_);
    for (uint32_t i = 0; i < framebuffers_.size(); i++) {
      frame_buffer_create_info.pAttachments = &images_[i].ImageView;
      vkCreateFramebuffer(device_->GetVulkanDevice(),
                          &frame_buffer_create_info, nullptr,
                          &framebuffers_[i]);
    }
  }
}

void VulkanSwapChain::Destroy()
{
  instance_ = nullptr;
}

void VulkanSwapChain::OnResize(uint32_t width, uint32_t height) {
  auto device = device_->GetVulkanDevice();
  vkDeviceWaitIdle(device);
  Create(width, height, v_sync_);
  vkDeviceWaitIdle(device);
}

void VulkanSwapChain::BeginFrame()
{
  auto& queue = Renderer::GetRenderResourceReleaseQueue(current_buffer_index_);
  queue.Execute();
	current_image_index_ = AcquireNextImage();
  vkResetCommandPool(device_->GetVulkanDevice(),
                     command_buffers_[current_buffer_index_].CommandPool, 0);
}

void VulkanSwapChain::Present() {
  constexpr VkPipelineStageFlags waitStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pWaitDstStageMask = &waitStageMask;
  submitInfo.pWaitSemaphores = &semaphores_.PresentComplete;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &semaphores_.RenderComplete;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pCommandBuffers =
      &command_buffers_[current_buffer_index_].CommandBuffer;
  submitInfo.commandBufferCount = 1;

  vkResetFences(device_->GetVulkanDevice(), 1,
                &wait_fences_[current_buffer_index_]);
  auto graphicsQueue = device_->GetGraphicsQueue();
  vkQueueSubmit(graphicsQueue, 1, &submitInfo,
                wait_fences_[current_buffer_index_]);

  VkResult result;
  {
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swap_chain_;
    presentInfo.pImageIndices = &current_image_index_;

    presentInfo.pWaitSemaphores = &semaphores_.RenderComplete;
    presentInfo.waitSemaphoreCount = 1;
    result = vkQueuePresentKHR(device_->GetGraphicsQueue(), &presentInfo);
  }

  if (result != VK_SUCCESS) {
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      OnResize(width_, height_);
    } else {
    }
  }

  {
    current_buffer_index_ = (current_buffer_index_ + 1) % 3;
    // Make sure the frame we're requesting has finished rendering
    vkWaitForFences(device_->GetVulkanDevice(), 1,
                    &wait_fences_[current_buffer_index_], VK_TRUE, UINT64_MAX);
  }
}

uint32_t VulkanSwapChain::AcquireNextImage() {
  uint32_t image_index;
  VkResult result =
      vkAcquireNextImageKHR(device_->GetVulkanDevice(), swap_chain_,
                            UINT64_MAX, semaphores_.PresentComplete,
                            reinterpret_cast<VkFence>(nullptr), &image_index);
  if (result != VK_SUCCESS) {
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      OnResize(width_, height_);
      vkAcquireNextImageKHR(device_->GetVulkanDevice(), swap_chain_,
                            UINT64_MAX, semaphores_.PresentComplete,
                            (VkFence) nullptr, &image_index);
    }
  }

  return image_index;
}

void VulkanSwapChain::SelectSurfaceFormat(const VkFormat select_format) {
  const VkPhysicalDevice physical_device = device_->GetVulkanPhysicalDevice();

  uint32_t surface_format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface_,
                                       &surface_format_count, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(surface_format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface_,
                                       &surface_format_count, formats.data());

  // 検索して一致するフォーマットを見つける.
  for (const auto& [format, colorSpace] : formats) {
    if (format == select_format) {
      color_format_ = format;
      color_space_ = colorSpace;
    }
  }
}
}  // namespace base_engine
