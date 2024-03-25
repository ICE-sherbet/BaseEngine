#include "DescriptorSet.h"

#include "VulkanImage.h"

namespace base_engine {
DescriptorSet::DescriptorSet() {}

void DescriptorSet::addBinding(uint32_t binding, uint32_t count,
                               VkDescriptorType type,
                               VkShaderStageFlags stage) {
  if (m_set) {
    BE_FATAL("DescriptorSet already generated, cannot add more bindings");
  }

  if (m_bindings.find(binding) != m_bindings.end()) {
    BE_WARN("Rebinding descriptor {}", binding);
  }

  VkDescriptorSetLayoutBinding b = {};
  b.binding = binding;
  b.descriptorCount = count;
  b.descriptorType = type;
  b.stageFlags = stage;

  m_bindings[binding] = b;
}

void DescriptorSet::generate() {
  m_pool = generatePool();

  m_layout = generateLayout();

  m_set = generateSet();
}

void DescriptorSet::bind(uint32_t binding, const Ref<VulkanBuffer>& buffer) {
  auto write = writeFromBinding(binding);

  write.pBufferInfo = &buffer->DescriptorInfo();
  m_pendingWrites.push_back(write);
}

void DescriptorSet::bind(uint32_t binding, const Ref<VulkanImage2D>& image) {
  auto write = writeFromBinding(binding);
  write.pImageInfo = &image->GetDescriptorInfoVulkan();

  m_pendingWrites.push_back(write);
}

void DescriptorSet::bind(uint32_t binding,
                         const Ref<TopLevelAS>& accelerationStructure) {
  auto write = writeFromBinding(binding);
  write.pNext = &accelerationStructure->descriptorInfo();

  m_pendingWrites.push_back(write);
}

void DescriptorSet::bind(VkWriteDescriptorSet write) {
  m_pendingWrites.push_back(write);
}

void DescriptorSet::update() {
  auto vkDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
  vkUpdateDescriptorSets(vkDevice, m_pendingWrites.size(),
                         m_pendingWrites.data(), 0, nullptr);
  m_pendingWrites.clear();
}

VkWriteDescriptorSet DescriptorSet::writeFromBinding(uint32_t index) const {
  const auto& binding = m_bindings.at(index);

  VkWriteDescriptorSet write = {};
  write.dstSet = m_set;
  write.dstBinding = index;
  write.descriptorType = binding.descriptorType;
  write.descriptorCount = binding.descriptorCount;

  return write;
}

VkDescriptorPool DescriptorSet::generatePool() const {
  constexpr auto toSize = [](auto it) {
    return VkDescriptorPoolSize{it.second.descriptorType,
                                it.second.descriptorCount};
  };

  std::vector<VkDescriptorPoolSize> sizes(m_bindings.size());
  std::ranges::transform(m_bindings, sizes.begin(), toSize);

  VkDescriptorPoolCreateInfo info = {};
  info.poolSizeCount = (uint32_t)sizes.size();
  info.pPoolSizes = sizes.data();
  info.maxSets = 1;
  auto vkDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
  VkDescriptorPool pool;
  vkCreateDescriptorPool(vkDevice, &info, nullptr, &pool);
  return pool;
}

VkDescriptorSetLayout DescriptorSet::generateLayout() const {
  std::vector<VkDescriptorSetLayoutBinding> bindings(m_bindings.size());
  std::ranges::transform(m_bindings, bindings.begin(),
                         [](auto it) { return it.second; });

  VkDescriptorSetLayoutCreateInfo info = {};
  info.bindingCount = (uint32_t)bindings.size();
  info.pBindings = bindings.data();
  VkDescriptorSetLayout layout;
  auto vkDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
  vkCreateDescriptorSetLayout(vkDevice, &info, nullptr, &layout);
  return layout;
}

VkDescriptorSet DescriptorSet::generateSet() const {
  VkDescriptorSetAllocateInfo info = {};
  info.descriptorPool = m_pool;
  info.descriptorSetCount = 1;
  info.pSetLayouts = &m_layout;

  auto vkDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
  auto set = VkDescriptorSet{};
  vkAllocateDescriptorSets(vkDevice, &info, &set);
  return set;
}

void DescriptorSet::setName(std::string_view name) {}

}  // namespace base_engine
