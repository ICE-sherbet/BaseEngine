// @DescriptorSet.h
// @brief
// @author ICE
// @date 2024/03/15
//
// @details

#pragma once

#include <cstdint>
#include <unordered_map>

#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "TopLevelAS.h"

namespace base_engine {
class DescriptorSet {
 public:
  DescriptorSet();

  /// Registers a new binding. Can only be used before the descriptor set has
  /// been generated.
  void addBinding(uint32_t binding, uint32_t count, VkDescriptorType type,
                  VkShaderStageFlags stage);

  /// Generates pool, layout, and set with the registered bindings.
  void generate();

  /// Record a binding request to be executed on the next `update` call.
  void bind(uint32_t binding, const Ref<VulkanBuffer>& buffer);
  void bind(uint32_t binding, const Ref<VulkanImage2D>& image);

  void bind(uint32_t binding, const Ref<TopLevelAS>& accelerationStructure);
  void bind(VkWriteDescriptorSet write);

  /// Executes all pending binding requests.
  void update();

  VkWriteDescriptorSet writeFromBinding(uint32_t binding) const;

  const VkDescriptorPool& pool() const { return m_pool; }

  const VkDescriptorSetLayout& layout() const { return m_layout; }

  const VkDescriptorSet& set() const { return m_set; }

  void setName(std::string_view name);

private:

  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

  VkDescriptorPool m_pool = VK_NULL_HANDLE;
  VkDescriptorPool generatePool() const;

  VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
  VkDescriptorSetLayout generateLayout() const;

  VkDescriptorSet m_set = VK_NULL_HANDLE;
  VkDescriptorSet generateSet() const;

  std::vector<VkWriteDescriptorSet> m_pendingWrites;
};
}  // namespace base_engine
