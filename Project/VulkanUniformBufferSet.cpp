#include "VulkanUniformBufferSet.h"

namespace base_engine {
VulkanUniformBufferSet::VulkanUniformBufferSet(uint32_t size,
                                               uint32_t framesInFlight)
    : m_FramesInFlight(framesInFlight) {
  if (framesInFlight == 0) m_FramesInFlight = 3;

  for (uint32_t frame = 0; frame < m_FramesInFlight; frame++)
    m_UniformBuffers[frame] = UniformBuffer::Create(size);
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
  BE_CORE_ASSERT(!m_UniformBuffers.contains(frame));
  return m_UniformBuffers.at(frame);
}

void VulkanUniformBufferSet::Set(Ref<UniformBuffer> uniformBuffer,
                                 uint32_t frame) {
  m_UniformBuffers[frame] = uniformBuffer;
}
}  // namespace base_engine
