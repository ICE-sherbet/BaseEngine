﻿// @file Resource.h
// @brief
// @author ICE
// @date 2022/09/24
//
// @details

#pragma once
#include <memory>
#include <unordered_map>
namespace asset_system {
using ResourceKey = size_t;
template <class Type>
using ResourceValue = std::shared_ptr<Type>;
template <class Type>
class Resource {
  using ResourceValue = ResourceValue<Type>;

 public:
  /**
   * \brief リソースを登録する
   * \tparam _Types リソースのコンストラクタに必要な引数型
   * \param key 登録キー
   * \param _Args リソースのコンストラクタの引数
   * \return 登録したリソース
   */
  template <typename... _Types>
  ResourceValue Register(const ResourceKey& key, _Types&&... _Args) {
    if constexpr (std::is_abstract_v<Type>) {
      resources_[key] = std::shared_ptr<Type>(_Args...);
    }else
    {
      resources_[key] = std::make_shared<Type>(_Args...);
    }

    return resources_[key];
  }
  ResourceValue Register(const ResourceKey& key, ResourceValue& resource) {
    resources_[key] = resource;
    return resource;
  }
  ResourceValue Get(const ResourceKey& key) {
    auto it = resources_.find(key);
    if (it != resources_.end()) {
      return it->second;
    }
    return std::shared_ptr<Type>();
  }

  [[nodiscard]] size_t GetSize() const { return resources_.size(); }

 private:
  std::unordered_map<ResourceKey, ResourceValue> resources_;
};
template <class Type>
using ResourcePtr = std::shared_ptr<Resource<Type>>;
}  // namespace asset_system