// @ComponentProperty.h
// @brief
// @author ICE
// @date 2023/06/25
//
// @details

#pragma once
#include <string_view>

#include "Becs/Registry.h"
template <typename T>
concept BE_CONCEPT_HasBind = requires(T) { T::_Bind(); };

#define BE_COMPONENT(class_)                                              \
  static inline const char* _GetClassNameStatic() { return #class_; }     \
  static inline void _Initialize() {                                      \
    base_engine::ComponentDB::AddClass<class_>();                         \
  }                                                                       \
  static inline id_type _GetHash() { return becs::type_hash<class_>(); }  \
  static inline size_t _GetSize() { return sizeof(class_); }              \
  static inline void _CreateRegistryPool(                                 \
      std::shared_ptr<becs::basic_sparse_set<>>& map,                     \
      std::allocator<becs::Entity> allocator) {                           \
    map = std::allocate_shared<becs::registry::storage_for_type<class_>>( \
        allocator, allocator);                                            \
  }                                                                       \
  constexpr void _CompileCheck() {                                        \
    static_assert(BE_CONCEPT_HasBind<class_>,                             \
                  #class_##"::_Bindメソッドを追加してください");          \
  }

#define BIND_PTR(property)

#define ADD_PROPERTY(info, set, get) \
  base_engine::ComponentDB::AddProperty(_GetClassNameStatic(), info, set, get)
#define REQUIRE_COMPONENT(target)                           \
  base_engine::ComponentDB::AddRequireComponent(_GetHash(), \
                                                target## ::_GetHash())
