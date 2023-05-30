// @ShapeComponents.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once

namespace base_engine::physics {
enum ShapeType { kShapeTypeNone, kCircle };

struct Circle {
 public:
  float radius;

  /**
   * \brief 質量慣性モーメントを求める\n
   * 回転のしにくさを表す
   * \param radius 半径
   * \param mass 質量
   * \return 質量慣性モーメント
   */
  static float CalculateMMOI(const float radius, const float mass) {
    return 0.5f * mass * radius * radius;
  }

  static constexpr size_t Type() { return ShapeType::kCircle; }
};
}  // namespace base_engine::physics