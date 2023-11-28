// @VulkanSwapChain.h
// @brief
// @author ICE
// @date 2023/10/31
//
// @details

#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "Assert.h"
#include "VulkanDevice.h"
namespace base_engine {
class VulkanSwapChain {
 public:
  VulkanSwapChain();
  ;

  static VulkanSwapChain* Get() { return instance_; }

  void Init(VkInstance instance, const Ref<VulkanDevice>& device);
  void InitSurface(GLFWwindow* window_handle);
  void Create(GLFWwindow* window_handle, bool vsync);
  void Create(uint32_t width, uint32_t height, bool vsync);
  void Destroy();

  void OnResize(uint32_t width, uint32_t height);

  void BeginFrame();
  void Present();

  uint32_t GetImageCount() const { return image_count_; }

  uint32_t GetWidth() const { return width_; }
  uint32_t GetHeight() const { return height_; }

  VkRenderPass GetRenderPass() { return render_pass_; }

  VkFramebuffer GetCurrentFramebuffer() {
    return GetFramebuffer(current_image_index_);
  }
  VkCommandBuffer GetCurrentDrawCommandBuffer() {
    return GetDrawCommandBuffer(current_buffer_index_);
  }

  VkFormat GetColorFormat() { return color_format_; }

  uint32_t GetCurrentBufferIndex() const { return current_buffer_index_; }

  VkFramebuffer GetFramebuffer(uint32_t index) {
    BE_CORE_ASSERT(index < framebuffers_.size());
    return framebuffers_[index];
  }

  VkCommandBuffer GetDrawCommandBuffer(uint32_t index) {
    BE_CORE_ASSERT(index < command_buffers_.size());
    return command_buffers_[index].CommandBuffer;
  }

  VkSemaphore GetRenderCompleteSemaphore() {
    return semaphores_.RenderComplete;
  }

  void SetVSync(const bool enabled) { v_sync_ = enabled; }

 private:
  uint32_t AcquireNextImage();

  void SelectSurfaceFormat(VkFormat select_format);

 private:
  VkInstance vk_instance_ = nullptr;
  Ref<VulkanDevice> device_;
  bool v_sync_ = false;

  VkFormat color_format_;
  VkColorSpaceKHR color_space_;

  VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
  uint32_t image_count_ = 0;
  std::vector<VkImage> vulkan_images_;

  struct SwapchainImage {
    VkImage Image = VK_NULL_HANDLE;
    VkImageView ImageView = VK_NULL_HANDLE;
  };
  std::vector<SwapchainImage> images_;

  std::vector<VkFramebuffer> framebuffers_;

  struct SwapchainCommandBuffer {
    VkCommandPool CommandPool = VK_NULL_HANDLE;
    VkCommandBuffer CommandBuffer = nullptr;
  };
  std::vector<SwapchainCommandBuffer> command_buffers_;

  struct {
    VkSemaphore PresentComplete = VK_NULL_HANDLE;
    VkSemaphore RenderComplete = VK_NULL_HANDLE;
  } semaphores_;
  VkSubmitInfo submit_info_;

  std::vector<VkFence> wait_fences_;

  VkRenderPass render_pass_ = VK_NULL_HANDLE;
  uint32_t current_buffer_index_ = 0;
  uint32_t current_image_index_ = 0;

  uint32_t queue_node_index_ = UINT32_MAX;
  uint32_t width_ = 0, height_ = 0;

  VkSurfaceKHR surface_;
  VkExtent2D extent_;
  friend class VulkanContext;

  static VulkanSwapChain* instance_;
};
}  // namespace base_engine
