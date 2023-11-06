// @VulkanContext.h
// @brief
// @author ICE
// @date 2023/10/30
//
// @details

#pragma once
#include <vulkan/vulkan.h>

#include "BaseEngineCore.h"
#include "BaseEngineRenderer.h"
#include "RendererContext.h"
#include "VulkanDevice.h"

namespace base_engine {
class VulkanContext : public RendererContext {
 public:
  void Init() override;
  ~VulkanContext() override;
  Ref<VulkanDevice> GetDevice() { return device_; }

  static VkInstance GetVkInstance() { return instance_; }

  static Ref<VulkanContext> Get() {
    return  Ref<VulkanContext>(BASE_ENGINE(RendererContext)->GetRendererContext());
  }

  static Ref<VulkanDevice> GetCurrentDevice() { return Get()->GetDevice(); }

 private:
  void InitInstance();

  Ref<VulkanPhysicalDevice> physics_device_;
  Ref<VulkanDevice> device_;

  static inline VkInstance instance_;


  VkPipelineCache pipeline_cache_ = 0;
  
};
}  // namespace base_engine
