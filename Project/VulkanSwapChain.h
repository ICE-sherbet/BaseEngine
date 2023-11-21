// @VulkanSwapChain.h
// @brief
// @author ICE
// @date 2023/10/31
//
// @details

#pragma once
#define WIN32_LEAN_AND_MEAN
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
  VulkanSwapChain() = default;

  static VulkanSwapChain* Get() { return instance_; }

  void Init(VkInstance instance, const Ref<VulkanDevice>& device);
  void InitSurface(GLFWwindow* window_handle);
  void Create(GLFWwindow* window_handle, bool vsync);
  void Create(uint32_t width, uint32_t height, bool vsync);
  void Destroy();

  void OnResize(uint32_t width, uint32_t height);

  void BeginFrame();
  void Present();

  uint32_t GetImageCount() const { return m_ImageCount; }

  uint32_t GetWidth() const { return m_Width; }
  uint32_t GetHeight() const { return m_Height; }

  VkRenderPass GetRenderPass() { return m_RenderPass; }

  VkFramebuffer GetCurrentFramebuffer() {
    return GetFramebuffer(m_CurrentImageIndex);
  }
  VkCommandBuffer GetCurrentDrawCommandBuffer() {
    return GetDrawCommandBuffer(m_CurrentBufferIndex);
  }

  VkFormat GetColorFormat() { return m_ColorFormat; }

  uint32_t GetCurrentBufferIndex() const { return m_CurrentBufferIndex; }

  VkFramebuffer GetFramebuffer(uint32_t index) {
    BE_CORE_ASSERT(index < m_Framebuffers.size());
    return m_Framebuffers[index];
  }

  VkCommandBuffer GetDrawCommandBuffer(uint32_t index) {
    BE_CORE_ASSERT(index < m_CommandBuffers.size());
    return m_CommandBuffers[index].CommandBuffer;
  }

  VkSemaphore GetRenderCompleteSemaphore() {
    return m_Semaphores.RenderComplete;
  }

  void SetVSync(const bool enabled) { m_VSync = enabled; }

 private:
  uint32_t AcquireNextImage();

  void SelectSurfaceFormat(VkFormat select_format);

 private:
  VkInstance m_Instance = nullptr;
  Ref<VulkanDevice> m_Device;
  bool m_VSync = false;

  VkFormat m_ColorFormat;
  VkColorSpaceKHR m_ColorSpace;

  VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
  uint32_t m_ImageCount = 0;
  std::vector<VkImage> m_VulkanImages;

  struct SwapchainImage {
    VkImage Image = VK_NULL_HANDLE;
    VkImageView ImageView = VK_NULL_HANDLE;
  };
  std::vector<SwapchainImage> m_Images;

  std::vector<VkFramebuffer> m_Framebuffers;

  struct SwapchainCommandBuffer {
    VkCommandPool CommandPool = VK_NULL_HANDLE;
    VkCommandBuffer CommandBuffer = nullptr;
  };
  std::vector<SwapchainCommandBuffer> m_CommandBuffers;

  struct {
    VkSemaphore PresentComplete = VK_NULL_HANDLE;
    VkSemaphore RenderComplete = VK_NULL_HANDLE;
  } m_Semaphores;
  VkSubmitInfo m_SubmitInfo;

  std::vector<VkFence> m_WaitFences;

  VkRenderPass m_RenderPass = VK_NULL_HANDLE;
  uint32_t m_CurrentBufferIndex = 0;
  uint32_t m_CurrentImageIndex = 0;

  uint32_t m_QueueNodeIndex = UINT32_MAX;
  uint32_t m_Width = 0, m_Height = 0;

  VkSurfaceKHR m_Surface;
  VkExtent2D m_extent;
  friend class VulkanContext;

  static VulkanSwapChain* instance_;
};
}  // namespace base_engine
