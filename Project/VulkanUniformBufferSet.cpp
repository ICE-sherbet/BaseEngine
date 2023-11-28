#include "VulkanUniformBufferSet.h"

namespace base_engine {
VulkanUniformBufferSet::VulkanUniformBufferSet(uint32_t size,
                                               uint32_t framesInFlight)
    : frames_in_flight_(framesInFlight) {
  if (framesInFlight == 0) frames_in_flight_ = 3;

  for (uint32_t frame = 0; frame < frames_in_flight_; frame++)
    uniform_buffers_[frame] = UniformBuffer::Create(size);
}

VulkanUniformBufferSet::~VulkanUniformBufferSet() {}

Ref<UniformBuffer> VulkanUniformBufferSet::Get() {
  const uint32_t frame = Renderer::GetCurrentFrameIndex();
  return Get(frame);
}

Ref<UniformBuffer> VulkanUniformBufferSet::RT_Get() {
  const uint32_t frame = Renderer::RT_GetCurrentFrameIndex();
  return Get(frame);
}

Ref<UniformBuffer> VulkanUniformBufferSet::Get(uint32_t frame) {
  BE_CORE_ASSERT(uniform_buffers_.contains(frame));
  return uniform_buffers_.at(frame);
}

void VulkanUniformBufferSet::Set(Ref<UniformBuffer> uniformBuffer,
                                 uint32_t frame) {
  uniform_buffers_[frame] = uniformBuffer;
}
}  // namespace base_engine
