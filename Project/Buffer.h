// @Buffer.h
// @brief
// @author ICE
// @date 2023/11/14
//
// @details

#pragma once
#include <cstdint>
#include <cstring>

#include "Assert.h"

namespace base_engine {
struct Buffer {
  using Byte = uint8_t;

  void* Data;
  uint64_t Size;

  Buffer() : Data(nullptr), Size(0) {}

  explicit Buffer(const void* data, const uint64_t size = 0)
      : Data(const_cast<void*>(data)), Size(size) {}

  static Buffer Copy(const Buffer& other) {
    Buffer buffer;
    buffer.Allocate(other.Size);
    memcpy(buffer.Data, other.Data, other.Size);
    return buffer;
  }

  static Buffer Copy(const void* data, const uint64_t size) {
    Buffer buffer;
    buffer.Allocate(size);
    memcpy(buffer.Data, data, size);
    return buffer;
  }

  void Allocate(const uint64_t size) {
    delete[] static_cast<Byte*>(Data);
    Data = nullptr;

    if (size == 0) return;

    Data = new Byte[size];
    Size = size;
  }

  void Release() {
    delete[] static_cast<Byte*>(Data);
    Data = nullptr;
    Size = 0;
  }

  void ZeroInitialize() const {
    if (Data) memset(Data, 0, Size);
  }

  template <typename T>
  T& Read(uint64_t offset = 0) {
    return *(T*)((Byte*)Data + offset);
  }

  template <typename T>
  const T& Read(uint64_t offset = 0) const {
    return *(T*)((Byte*)Data + offset);
  }

  Byte* ReadBytes(uint64_t size, uint64_t offset) const {
    BE_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
    Byte* buffer = new Byte[size];
    memcpy(buffer, static_cast<Byte*>(Data) + offset, size);
    return buffer;
  }

  void Write(const void* data, uint64_t size, uint64_t offset = 0) const {
    BE_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
    memcpy((Byte*)Data + offset, data, size);
  }

  explicit operator bool() const { return Data; }

  Byte& operator[](const int index) { return static_cast<Byte*>(Data)[index]; }

  Byte operator[](const int index) const {
    return static_cast<Byte*>(Data)[index];
  }

  template <typename T>
  T* As() const {
    return (T*)Data;
  }

  inline uint64_t GetSize() const { return Size; }
};

struct BufferSafe : public Buffer {
  ~BufferSafe() { Release(); }

  static BufferSafe Copy(const void* data, const uint64_t size) {
    BufferSafe buffer;
    buffer.Allocate(size);
    memcpy(buffer.Data, data, size);
    return buffer;
  }
};
}  // namespace base_engine