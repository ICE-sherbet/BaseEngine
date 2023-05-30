// @MonoGCHandle.h
// @brief
// @author ICE
// @date 2023/04/07
//
// @details

#pragma once
#include <cstddef>
#include <unordered_map>

namespace base_engine {

enum class GCHandleType : char { NIL, STRONG_HANDLE, WEAK_HANDLE };

extern "C" {
struct GCHandle {
  void *value;

  bool operator==(const GCHandle &p_other) const {
    return value == p_other.value;
  }
  bool operator!=(const GCHandle &p_other) const {
    return value != p_other.value;
  }

  GCHandle() : value(nullptr){};
  GCHandle(std::nullptr_t) : value(nullptr) {}
  GCHandle(void *pointer) : value(pointer) {}
};
}

static_assert(sizeof(GCHandle) == sizeof(void *));

struct MonoGCHandleData {
  GCHandle handle = nullptr;
  GCHandleType type = GCHandleType::NIL;

  [[nodiscard]] bool IsReleased() const { return !handle.value; }
  [[nodiscard]] bool IsWeak() const {
    return type == GCHandleType::WEAK_HANDLE;
  }
  [[nodiscard]] GCHandle GetIntPtr() const { return handle; }

  void Release() {}

  static void FreeGChandle(GCHandle p_gchandle) {}

  void operator=(const MonoGCHandleData &p_other) {
    handle = p_other.handle;
    type = p_other.type;
  }

  MonoGCHandleData() = default;
  MonoGCHandleData(const MonoGCHandleData &) = default;
  MonoGCHandleData(const GCHandle p_handle, const GCHandleType p_type)
      : handle(p_handle), type(p_type) {}
};

}  // namespace base_engine

namespace std {
template <>
struct hash<base_engine::GCHandle> {
  size_t operator()(const base_engine::GCHandle& handle) const noexcept { return reinterpret_cast<size_t>(handle.value); }
};
}  // namespace std