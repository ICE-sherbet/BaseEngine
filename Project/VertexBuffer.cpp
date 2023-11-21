#include "VertexBuffer.h"

#include "RendererApi.h"
#include "VulkanVertexBuffer.h"

namespace base_engine {

namespace {
constexpr uint32_t ShaderDataTypeSize(const ShaderDataType type) {
  switch (type) {
    case ShaderDataType::kInt:
      return 4;
    case ShaderDataType::kInt2:
      return 4 * 2;
    case ShaderDataType::kInt3:
      return 4 * 3;
    case ShaderDataType::kInt4:
      return 4 * 4;
    case ShaderDataType::kFloat:
      return 4;
    case ShaderDataType::kFloat2:
      return 4 * 2;
    case ShaderDataType::kFloat3:
      return 4 * 3;
    case ShaderDataType::kFloat4:
      return 4 * 4;
    case ShaderDataType::kMatrix3:
      return 4 * 3 * 3;
    case ShaderDataType::kMatrix4:
      return 4 * 4 * 4;
    case ShaderDataType::kBool:
      return 1;
  }
  return 0;
}

}  // namespace

VertexBufferElement::VertexBufferElement() {}

VertexBufferElement::VertexBufferElement(const ShaderDataType type,
                                         const std::string& name,
                                         const bool normalized)
    : name(name),
      type(type),
      size(ShaderDataTypeSize(type)),
      offset(0),
      normalized(normalized) {}

uint32_t VertexBufferElement::GetElementCount() const {
  switch (type) {
    case ShaderDataType::kFloat:
      return 1;
    case ShaderDataType::kFloat2:
      return 2;
    case ShaderDataType::kFloat3:
      return 3;
    case ShaderDataType::kFloat4:
      return 4;
    case ShaderDataType::kMatrix3:
      return 3 * 3;
    case ShaderDataType::kMatrix4:
      return 4 * 4;
    case ShaderDataType::kInt:
      return 1;
    case ShaderDataType::kInt2:
      return 2;
    case ShaderDataType::kInt3:
      return 3;
    case ShaderDataType::kInt4:
      return 4;
    case ShaderDataType::kBool:
      return 1;
  }

  return 0;
}

VertexBufferLayout::VertexBufferLayout() {}

VertexBufferLayout::VertexBufferLayout(
    const std::initializer_list<VertexBufferElement>& elements)
    : elements_(elements) {
  CalculateOffsetsAndStride();
}

uint32_t VertexBufferLayout::GetStride() const { return stride_; }

const std::vector<VertexBufferElement>& VertexBufferLayout::GetElements()
    const {
  return elements_;
}

uint32_t VertexBufferLayout::GetElementCount() const {
  return elements_.size();
}

VertexBufferLayout::Iterator VertexBufferLayout::begin() {
  return elements_.begin();
}

VertexBufferLayout::Iterator VertexBufferLayout::end() {
  return elements_.end();
}

VertexBufferLayout::ConstIterator VertexBufferLayout::begin() const {
  return elements_.begin();
}

VertexBufferLayout::ConstIterator VertexBufferLayout::end() const {
  return elements_.end();
}

void VertexBufferLayout::CalculateOffsetsAndStride() {
  uint32_t offset = 0;
  stride_ = 0;
  for (auto& element : elements_) {
    element.offset = offset;
    offset += element.size;
    stride_ += element.size;
  }
}

Ref<VertexBuffer> VertexBuffer::Create(void* data, uint32_t size,
                                       VertexBufferUsage usage) {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return Ref<VulkanVertexBuffer>::Create(data, size, usage);
  }
}

Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, VertexBufferUsage usage) {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return Ref<VulkanVertexBuffer>::Create(size, usage);
  }
}
}  // namespace base_engine
