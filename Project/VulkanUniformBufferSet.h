// @VulkanUniformBufferSet.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include <cstdint>
#include <map>

#include "Assert.h"
#include "RendererApi.h"
#include "UniformBuffer.h"
#include "UniformBufferSet.h"

namespace base_engine {
class VulkanUniformBufferSet : public UniformBufferSet {
 public:
  VulkanUniformBufferSet(uint32_t size, uint32_t framesInFlight);

  ~VulkanUniformBufferSet() override;

  virtual Ref<UniformBuffer> Get() override;

  virtual Ref<UniformBuffer> RT_Get() override;

  virtual Ref<UniformBuffer> Get(uint32_t frame) override;

  virtual void Set(Ref<UniformBuffer> uniformBuffer,
                   uint32_t frame = 0) override;

private:
  uint32_t frames_in_flight_ = 0;
  std::map<uint32_t, Ref<UniformBuffer>> uniform_buffers_;
};
}  // namespace base_engine
