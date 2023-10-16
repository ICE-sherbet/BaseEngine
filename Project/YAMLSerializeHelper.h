// @YAMLSerializeHelper.h
// @brief
// @author ICE
// @date 2023/05/18
//
// @details

#pragma once
#include "Quaternion.h"
#include "Vector3.h"
#include "yaml-cpp/node/node.h"

namespace YAML {
#define YAML_MAKE_NVP(node, instance, field) \
  (node##[#field] = ##instance.##field)
#define YAML_GET_NVP(node, instance, field) \
  (##instance.##field = node##[#field])
#define YAML_GET_NVP_AS(node, instance, field, type) \
  (##instance.##field = node##[#field].as<type>())

#define YAML_GET_NVP_AUTO(node, instance, field) \
  (##instance.##field = node##[#field].as<decltype(##instance.##field)>())

template <>
struct convert<base_engine::Vector2> {
  static Node encode(const base_engine::Vector2& rhs) {
    Node node;
    YAML_MAKE_NVP(node, rhs, x);
    YAML_MAKE_NVP(node, rhs, y);
    return node;
  }

  static bool decode(const Node& node, base_engine::Vector2& rhs) {
    if (node.size() != 2) return false;

    YAML_GET_NVP_AUTO(node, rhs, x);
    YAML_GET_NVP_AUTO(node, rhs, y);
    return true;
  }
};
template <>
struct convert<base_engine::Vector3> {
  static Node encode(const base_engine::Vector3& rhs) {
    Node node;
    YAML_MAKE_NVP(node, rhs, x);
    YAML_MAKE_NVP(node, rhs, y);
    YAML_MAKE_NVP(node, rhs, z);
    return node;
  }

  static bool decode(const Node& node, base_engine::Vector3& rhs) {
    if (node.size() != 3) return false;

    YAML_GET_NVP_AUTO(node, rhs, x);
    YAML_GET_NVP_AUTO(node, rhs, y);
    YAML_GET_NVP_AUTO(node, rhs, z);
    return true;
  }
};

template <>
struct convert<base_engine::Vector4> {
  static Node encode(const base_engine::Vector4& rhs) {
    Node node;
    YAML_MAKE_NVP(node, rhs, w);
    YAML_MAKE_NVP(node, rhs, x);
    YAML_MAKE_NVP(node, rhs, y);
    YAML_MAKE_NVP(node, rhs, z);
    return node;
  }

  static bool decode(const Node& node, base_engine::Vector4& rhs) {
    if (node.size() != 4) return false;

    YAML_GET_NVP_AUTO(node, rhs, w);
    YAML_GET_NVP_AUTO(node, rhs, x);
    YAML_GET_NVP_AUTO(node, rhs, y);
    YAML_GET_NVP_AUTO(node, rhs, z);
    return true;
  }
};

template <>
struct convert<base_engine::Quaternion> {
  static Node encode(const base_engine::Quaternion& rhs) {
    Node node;
    YAML_MAKE_NVP(node, rhs, w);
    YAML_MAKE_NVP(node, rhs, x);
    YAML_MAKE_NVP(node, rhs, y);
    YAML_MAKE_NVP(node, rhs, z);
    return node;
  }

  static bool decode(const Node& node, base_engine::Quaternion& rhs) {
    if (node.size() != 4) return false;

    YAML_GET_NVP_AUTO(node, rhs, w);
    YAML_GET_NVP_AUTO(node, rhs, x);
    YAML_GET_NVP_AUTO(node, rhs, y);
    YAML_GET_NVP_AUTO(node, rhs, z);
    return true;
  }
};

template <>
struct convert<base_engine::AssetHandle> {
  static Node encode(const base_engine::AssetHandle& rhs) {
    Node node;
    node.push_back(static_cast<uint64_t>(rhs));
    return node;
  }

  static bool decode(const Node& node, base_engine::AssetHandle& rhs) {
    rhs = node.as<uint64_t>();
    return true;
  }
};

template <>
struct convert<base_engine::Variant> {
  static Node encode(const base_engine::Variant& rhs) {
    Node node;
    float buf[6];
    std::memcpy(buf, &rhs, sizeof(rhs));
    for (int i = 0; i < 6; ++i) {
      node.push_back(buf[i]);
    }
    return node;
  }

  static bool decode(const Node& node, base_engine::Variant& rhs) {
    auto type = node.as<int>();
    return true;
  }
};
}  // namespace YAML
