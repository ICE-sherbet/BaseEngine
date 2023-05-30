// @TypeUtilities.h
// @brief
// @author ICE
// @date 2023/04/13
//
// @details

#pragma once
#include <source_location>
#include <string_view>
namespace base_engine {
class TypeUtilities {
 public:
  TypeUtilities() = delete;

  template <typename Type>
  consteval static std::string_view GetFullTypeName() {
    constexpr std::source_location location = std::source_location::current();
    constexpr std::string_view function_name = location.function_name();
    constexpr std::size_t end = function_name.find(">(void)");
    constexpr std::size_t check = function_name.find("<struct");
    if (check != std::string_view::npos) {
      constexpr std::size_t begin_struct = function_name.find("<struct") + 8;
      return function_name.substr(begin_struct, end - begin_struct);
    } else {
      constexpr std::size_t begin_class = function_name.find("<class") + 7;
      return function_name.substr(begin_class, end - begin_class);
    }
  }
  template <typename Type, bool ExcludeNamespace = false>
  constexpr static std::string_view GetTypeName() {
	  constexpr std::string_view pre_class_name = GetFullTypeName<Type>();
    if constexpr (ExcludeNamespace) {
      constexpr auto begin_namespace = pre_class_name.find_last_of("::");
      constexpr std::string_view class_name =
          pre_class_name.substr(begin_namespace+1);
      return class_name;
    }
    return pre_class_name;
  }
};

}  // namespace base_engine