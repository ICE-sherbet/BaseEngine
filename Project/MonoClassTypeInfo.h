// @MonoClassTypeInfo.h
// @brief
// @author ICE
// @date 2023/04/06
//
// @details

#pragma once
#include <string>
#include <vector>

#include "ScriptTypes.h"
namespace base_engine {
namespace detail {

template <typename T>
inline void hash_combine(size_t& result, const T& value) {
  std::hash<T> h;
  result ^= h(value) + 0x9e3779b9 + (result << 6) + (result >> 2);
}

}  // namespace detail

using MonoClassHandle = uint32_t;
struct MonoClassTypeInfo {
  MonoClassHandle id = 0;
  std::string full_name;
  std::vector<uint32_t> fields;
  std::vector<uint32_t> methods;
  uint32_t size = 0;

  uint32_t parent_id = 0;
  MonoClass* mono_class = nullptr;
};
using MonoFieldHandle = uint32_t;
struct MonoFieldInfo {
  MonoClassHandle id = 0;
  PropertyInfo field_info;
  MonoType* type = nullptr;
  uint64_t attribute_flags = 0;
  MonoClassField* mono_field = nullptr;
  uint32_t size;
};

using MonoMethodHandle = uint32_t;
struct MonoMethodInfo {
  MonoClassHandle id = 0;
  MethodInfo method_info;
  uint32_t parameter_count = 0;
  MonoMethod* method = nullptr;
};
struct AssemblyMetadata {
  std::string Name;
  uint32_t MajorVersion;
  uint32_t MinorVersion;
  uint32_t BuildVersion;
  uint32_t RevisionVersion;

  bool operator==(const AssemblyMetadata& other) const {
    return Name == other.Name && MajorVersion == other.MajorVersion &&
           MinorVersion == other.MinorVersion &&
           BuildVersion == other.BuildVersion &&
           RevisionVersion == other.RevisionVersion;
  }

  bool operator!=(const AssemblyMetadata& other) const {
    return !(*this == other);
  }
};

struct AssemblyInfo : public RefCounted {
  std::filesystem::path FilePath = "";
  MonoAssembly* Assembly = nullptr;
  MonoImage* AssemblyImage = nullptr;
  std::vector<uint32_t> Classes;
  bool IsCoreAssembly = false;
  AssemblyMetadata Metadata;
  std::vector<AssemblyMetadata> ReferencedAssemblies;
};
}  // namespace base_engine

namespace std {

template <>
struct hash<base_engine::AssemblyMetadata> {
  size_t operator()(
      const base_engine::AssemblyMetadata& metadata) const noexcept {
    size_t result = 0;
    base_engine::detail::hash_combine(result, metadata.Name);
    base_engine::detail::hash_combine(result, metadata.MajorVersion);
    base_engine::detail::hash_combine(result, metadata.MinorVersion);
    base_engine::detail::hash_combine(result, metadata.BuildVersion);
    base_engine::detail::hash_combine(result, metadata.RevisionVersion);
    return result;
  }
};

}  // namespace std