// @VertexBuffer.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include <cstdint>

#include "Assert.h"
#include "Ref.h"

namespace base_engine {

enum class ShaderDataType {
  kNone = 0,
  kInt,
  kInt2,
  kInt3,
  kInt4,
  kFloat,
  kFloat2,
  kFloat3,
  kFloat4,
  kMatrix3,
  kMatrix4,
  kBool
};

struct VertexBufferElement {
  std::string name;
  ShaderDataType type;
  uint32_t size;
  uint32_t offset;
  bool normalized;

  VertexBufferElement();

  VertexBufferElement(ShaderDataType type, const std::string& name,
                      bool normalized = false);

  [[nodiscard]] uint32_t GetElementCount() const;
};

class VertexBufferLayout {
  using Iterator = std::vector<VertexBufferElement>::iterator;
  using ConstIterator = std::vector<VertexBufferElement>::const_iterator;

 public:
  VertexBufferLayout();

  VertexBufferLayout(
      const std::initializer_list<VertexBufferElement>& elements);

  [[nodiscard]] uint32_t GetStride() const;

  [[nodiscard]] const std::vector<VertexBufferElement>& GetElements() const;

  [[nodiscard]] uint32_t GetElementCount() const;

  [[nodiscard]] Iterator begin();

  [[nodiscard]] Iterator end();

  [[nodiscard]] ConstIterator begin() const;

  [[nodiscard]] ConstIterator end() const;

 private:
  void CalculateOffsetsAndStride();

 private:
  std::vector<VertexBufferElement> elements_;
  uint32_t stride_ = 0;
};

enum class VertexBufferUsage { kNone = 0, kStatic = 1, kDynamic = 2 };
class VertexBuffer : public RefCounted {
 public:
  virtual ~VertexBuffer() {}

  virtual void SetData(void* buffer, uint32_t size, uint32_t offset = 0) = 0;
  virtual void RT_SetData(void* buffer, uint32_t size, uint32_t offset = 0) = 0;
  virtual void Bind() const = 0;

  virtual unsigned int GetSize() const = 0;

  static Ref<VertexBuffer> Create(
      void* data, uint32_t size,
      VertexBufferUsage usage = VertexBufferUsage::kStatic);
  static Ref<VertexBuffer> Create(
      uint32_t size, VertexBufferUsage usage = VertexBufferUsage::kDynamic);
};
}  // namespace base_engine