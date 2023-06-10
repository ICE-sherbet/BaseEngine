// @file FrozenHelper.h
// @brief
// @author ICE
// @date 2022/10/02
//
// @details

#pragma once
#include <type_traits>
namespace frozen {
namespace detail {
struct NameValuePairCore {};
}  // namespace detail

template <class T>
class NameValuePair : detail::NameValuePairCore {
 private:
  using Type = std::conditional_t<
      std::is_array_v<std::remove_reference_t<T>>, std::remove_cv_t<T>,
      std::conditional_t<std::is_lvalue_reference_v<T>, T, std::decay_t<T>>>;

  static_assert(!std::is_base_of_v<detail::NameValuePairCore, T>,
                "ValueをNameValuePairCoreにして入れ子にすることは出来ない");

  NameValuePair& operator=(NameValuePair const&) = delete;

 public:
  NameValuePair(char const* n, T&& v) : name(n), value(std::forward<T>(v)) {}

  char const* name;
  Type value;
};

template <class T>
NameValuePair<T> make_name_value_pair(char const* name, T&& value) {
  return NameValuePair<T>{name, std::forward<T>(value)};
}

using SizeType = uint64_t;

template <class T>
class SizeTag {
 private:
  using Type =
      std::conditional_t<std::is_lvalue_reference_v<T>, T, std::decay_t<T>>;
  SizeTag& operator=(SizeTag const&) = delete;

 public:
  explicit SizeTag(T&& sz) : size_(std::forward<T>(sz)) {}

  Type size_;
};
template <class T>
struct BinaryData {
  using TPtr = std::conditional_t<
      std::is_const_v<std::remove_pointer_t<std::remove_reference_t<T>>>,
      const void*, void*>;

  BinaryData(T&& d, uint64_t s) : data(std::forward<T>(d)), size(s) {}

  TPtr data;
  uint64_t size;
};

template <class T>
BinaryData<T> binary_data(T&& data, size_t size) {
  return {std::forward<T>(data), size};
}

template <class T>
SizeTag<T> make_size_tag(T&& size) {
  return SizeTag<T>{std::forward<T>(size)};
}

}  // namespace frozen
template <class ArchiveType, class T>
concept BinaryArchiveConcept =
    requires(ArchiveType& ar, frozen::BinaryData<T> binary) { ar(binary); };
