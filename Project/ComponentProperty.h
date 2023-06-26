// @ComponentProperty.h
// @brief
// @author ICE
// @date 2023/06/25
//
// @details

#pragma once
#include <string_view>

#define BE_COMPONENT(class_)                                               \
  static inline const char* _GetClassNameStatic() { return #class_; } \
  static inline void _Initialize() {                                       \
    base_engine::ComponentDB::AddClass<class_>();                          \
    class_::_Bind();                                                       \
  }
#define BIND_PTR(property)

#define ADD_PROPERTY(info, set, get)                                        \
  base_engine::ComponentDB::AddProperty(_GetClassNameStatic(), info, \
                                        set, get)
