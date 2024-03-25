#define VMA_IMPLEMENTATION
#include "VulkanAllocator.h"

#include <ranges>

#include "Assert.h"
#include "VulkanContext.h"
#define BE_GPU_TRACK_MEMORY_ALLOCATION true;
#define HZ_LOG_RENDERER_ALLOCATIONS true;

#if HZ_LOG_RENDERER_ALLOCATIONS
#define BE_ALLOCATOR_LOG(...) BE_CORE_TRACE(__VA_ARGS__)
#else
#define HZ_ALLOCATOR_LOG(...)
#endif

namespace base_engine {
struct VulkanAllocatorData {
  VmaAllocator allocator;
  uint64_t total_allocated_bytes = 0;
  uint64_t memory_usage = 0;
};

enum class AllocationType : uint8_t { kNone = 0, kBuffer = 1, kImage = 2 };

static VulkanAllocatorData* s_Data = nullptr;

struct AllocInfo {
  uint64_t allocated_size = 0;
  AllocationType type = AllocationType::kNone;
};

static std::map<VmaAllocation, AllocInfo> s_AllocationMap;

namespace Utils {
std::string BytesToString(uint64_t bytes) {
  constexpr uint64_t GB = 1024 * 1024 * 1024;
  constexpr uint64_t MB = 1024 * 1024;
  constexpr uint64_t KB = 1024;

  char buffer[32 + 1]{};

  if (bytes > GB)
    snprintf(buffer, 32, "%.2f GB", (float)bytes / (float)GB);
  else if (bytes > MB)
    snprintf(buffer, 32, "%.2f MB", (float)bytes / (float)MB);
  else if (bytes > KB)
    snprintf(buffer, 32, "%.2f KB", (float)bytes / (float)KB);
  else
    snprintf(buffer, 32, "%.2f bytes", (float)bytes);

  return std::string(buffer);
}
}  // namespace Utils

VulkanAllocator::VulkanAllocator(const std::string& tag) : tag_(tag) {}

VulkanAllocator::~VulkanAllocator() {}

VmaAllocation VulkanAllocator::AllocateBuffer(
    const VkBufferCreateInfo& buffer_create_info, const VmaMemoryUsage usage,
    VkBuffer& out_buffer) {
  BE_CORE_VERIFY(buffer_create_info.size > 0);

  VmaAllocationCreateInfo alloc_create_info = {};
  alloc_create_info.usage = usage;

  VmaAllocation allocation;
  vmaCreateBuffer(s_Data->allocator, &buffer_create_info, &alloc_create_info,
                  &out_buffer, &allocation, nullptr);
  if (allocation == nullptr) {
    BE_CORE_ERROR_TAG("Renderer", "Failed to allocate GPU buffer!");
    BE_CORE_ERROR_TAG("Renderer", "  Requested size: {}",
                      Utils::BytesToString(buffer_create_info.size));
    auto stats = GetStats();
    BE_CORE_ERROR_TAG("Renderer", "  GPU mem usage: {}/{}",
                      Utils::BytesToString(stats.Used),
                      Utils::BytesToString(stats.TotalAvailable));
  }

  VmaAllocationInfo allocInfo{};
  vmaGetAllocationInfo(s_Data->allocator, allocation, &allocInfo);

  {
    s_Data->total_allocated_bytes += allocInfo.size;
    BE_CORE_TRACE("VulkanAllocator ({0}): total allocated since start is {1}",
                  tag_, Utils::BytesToString(s_Data->total_allocated_bytes));
  }

#if BE_GPU_TRACK_MEMORY_ALLOCATION
  auto& alloc_track = s_AllocationMap[allocation];
  alloc_track.allocated_size = allocInfo.size;
  alloc_track.type = AllocationType::kBuffer;
  s_Data->memory_usage += allocInfo.size;
#endif

  return allocation;
}

VmaAllocation VulkanAllocator::AllocateImage(
    VkImageCreateInfo image_create_info, VmaMemoryUsage usage,
    VkImage& out_image, VkDeviceSize* allocated_size) {
  VmaAllocationCreateInfo alloc_create_info = {};
  alloc_create_info.usage = usage;

  VmaAllocation allocation;
  vmaCreateImage(s_Data->allocator, &image_create_info, &alloc_create_info,
                 &out_image, &allocation, nullptr);
  if (allocation == nullptr) {
    BE_CORE_ERROR_TAG("Renderer", "Failed to allocate GPU image!");
    BE_CORE_ERROR_TAG("Renderer", "  Requested size: {}x{}x{}",
                      image_create_info.extent.width,
                      image_create_info.extent.height,
                      image_create_info.extent.depth);
    BE_CORE_ERROR_TAG("Renderer", "  Mips: {}", image_create_info.mipLevels);
    BE_CORE_ERROR_TAG("Renderer", "  Layers: {}",
                      image_create_info.arrayLayers);
    auto stats = GetStats();
    BE_CORE_ERROR_TAG("Renderer", "  GPU mem usage: {}/{}",
                      Utils::BytesToString(stats.Used),
                      Utils::BytesToString(stats.TotalAvailable));
  }

  VmaAllocationInfo allocInfo;
  vmaGetAllocationInfo(s_Data->allocator, allocation, &allocInfo);
  if (allocated_size) *allocated_size = allocInfo.size;
  BE_ALLOCATOR_LOG("VulkanAllocator ({0}): allocating image; size = {1}", tag_,
                   Utils::BytesToString(allocInfo.size));

  {
    s_Data->total_allocated_bytes += allocInfo.size;
    BE_ALLOCATOR_LOG(
        "VulkanAllocator ({0}): total allocated since start is {1}", tag_,
        Utils::BytesToString(s_Data->total_allocated_bytes));
  }

#if BE_GPU_TRACK_MEMORY_ALLOCATION
  auto& allocTrack = s_AllocationMap[allocation];
  allocTrack.allocated_size = allocInfo.size;
  allocTrack.type = AllocationType::kImage;
  s_Data->memory_usage += allocInfo.size;
#endif

  return allocation;
}

void VulkanAllocator::Free(const VmaAllocation allocation) {
  vmaFreeMemory(s_Data->allocator, allocation);

#if BE_GPU_TRACK_MEMORY_ALLOCATION
  auto it = s_AllocationMap.find(allocation);
  if (it != s_AllocationMap.end()) {
    s_Data->memory_usage -= it->second.allocated_size;
    s_AllocationMap.erase(it);
  } else {
    BE_CORE_ERROR("Could not find GPU memory allocation: {}",
                  (void*)allocation);
  }
#endif
}

void VulkanAllocator::DestroyImage(VkImage image, VmaAllocation allocation) {
  BE_CORE_ASSERT(image);
  BE_CORE_ASSERT(allocation);
  vmaDestroyImage(s_Data->allocator, image, allocation);

#if BE_GPU_TRACK_MEMORY_ALLOCATION
  auto it = s_AllocationMap.find(allocation);
  if (it != s_AllocationMap.end()) {
    s_Data->memory_usage -= it->second.allocated_size;
    s_AllocationMap.erase(it);
  } else {
    BE_CORE_ERROR("Could not find GPU memory allocation: {}",
                  (void*)allocation);
  }
#endif
}

void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation) {
  BE_CORE_ASSERT(buffer);
  BE_CORE_ASSERT(allocation);
  vmaDestroyBuffer(s_Data->allocator, buffer, allocation);

#if BE_GPU_TRACK_MEMORY_ALLOCATION
  auto it = s_AllocationMap.find(allocation);
  if (it != s_AllocationMap.end()) {
    s_Data->memory_usage -= it->second.allocated_size;
    s_AllocationMap.erase(it);
  } else {
    BE_CORE_ERROR("Could not find GPU memory allocation: {}",
                  (void*)allocation);
  }
#endif
}

void VulkanAllocator::UnmapMemory(VmaAllocation allocation) {
  vmaUnmapMemory(s_Data->allocator, allocation);
}

void VulkanAllocator::DumpStats() {
  const auto& memoryProps = VulkanContext::GetCurrentDevice()
                                ->GetPhysicalDevice()
                                ->GetMemoryProperties();
  std::vector<VmaBudget> budgets(memoryProps.memoryHeapCount);
  vmaGetBudget(s_Data->allocator, budgets.data());

  BE_CORE_WARN("-----------------------------------");
  for (VmaBudget& b : budgets) {
    BE_CORE_WARN("VmaBudget.allocationBytes = {0}",
                 Utils::BytesToString(b.allocationBytes));
    BE_CORE_WARN("VmaBudget.blockBytes = {0}",
                 Utils::BytesToString(b.blockBytes));
    BE_CORE_WARN("VmaBudget.usage = {0}", Utils::BytesToString(b.usage));
    BE_CORE_WARN("VmaBudget.budget = {0}", Utils::BytesToString(b.budget));
  }
  BE_CORE_WARN("-----------------------------------");
}

GPUMemoryStats VulkanAllocator::GetStats() {
  const auto& memoryProps = VulkanContext::GetCurrentDevice()
                                ->GetPhysicalDevice()
                                ->GetMemoryProperties();

  std::vector<VmaBudget> budgets(memoryProps.memoryHeapCount);
  vmaGetBudget(s_Data->allocator, budgets.data());

  uint64_t budget = 0;
  for (const VmaBudget& b : budgets) budget += b.budget;

  GPUMemoryStats result;
  for (const auto& [AllocatedSize, Type] :
       s_AllocationMap | std::views::values) {
    if (Type == AllocationType::kBuffer) {
      result.BufferAllocationCount++;
      result.BufferAllocationSize += AllocatedSize;
    } else if (Type == AllocationType::kImage) {
      // TODO Imageでの時のカウントを行う
    }
  }

  result.AllocationCount = s_AllocationMap.size();
  result.Used = s_Data->memory_usage;
  result.TotalAvailable = budget;
  return result;
}

void VulkanAllocator::Init(Ref<VulkanDevice> device) {
  s_Data = new VulkanAllocatorData();

  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
  allocatorInfo.physicalDevice = device->GetVulkanPhysicalDevice();
  allocatorInfo.device = device->GetVulkanDevice();
  allocatorInfo.instance = VulkanContext::GetVkInstance();

  vmaCreateAllocator(&allocatorInfo, &s_Data->allocator);
}

void VulkanAllocator::Shutdown() {
  vmaDestroyAllocator(s_Data->allocator);

  delete s_Data;
  s_Data = nullptr;
}

VmaAllocator& VulkanAllocator::GetVMAAllocator() { return s_Data->allocator; }
}  // namespace base_engine
