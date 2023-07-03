// @EditorDragValue.h
// @brief
// @author ICE
// @date 2023/06/27
//
// @details

#pragma once
#include <numeric>
#include <string>

#include "EditorControl.h"

namespace base_engine::editor {
class EditorDragValue : public EditorControl {
 public:
  EditorDragValue(const std::string& text = "", float value = 0,
                  float min = std::numeric_limits<float>::min(),
                  float max = std::numeric_limits<float>::max())
      : text_(text), value_(value), min_(min), max_(max) {}

  void Notification(int type) override;
  [[nodiscard]] float Value() const { return value_; }
  void SetValue(float value) { value_ = value; }

 private:
  std::string text_;
  float value_;
  float min_;
  float max_;
};

class EditorDragValue2 : public EditorControl {
 public:
  EditorDragValue2(const std::string& text = "", Vector2 value = {0, 0},
                   float min = std::numeric_limits<float>::min(),
                   float max = std::numeric_limits<float>::max())
      : text_(text), value_(value), min_(min), max_(max) {}

  void Notification(int type) override;

  [[nodiscard]] Vector2 Value() const { return value_; }
  void SetValue(Vector2 value) { value_ = value; }

 private:
  std::string text_;
  Vector2 value_;
  float min_;
  float max_;
};
class EditorDragValue3 : public EditorControl {
 public:
  EditorDragValue3(const std::string& text = "", Vector3 value = {0, 0, 0},
                   float min = std::numeric_limits<float>::min(),
                   float max = std::numeric_limits<float>::max())
      : text_(text), value_(value), min_(min), max_(max) {}

  void Notification(int type) override;

  [[nodiscard]] Vector3 Value() const { return value_; }
  void SetValue(Vector3 value) { value_ = value; }

 private:
  std::string text_;
  Vector3 value_;
  float min_;
  float max_;
};

class EditorDragValue4 : public EditorControl {
 public:
  EditorDragValue4(const std::string& text = "", Vector4 value = {0, 0, 0, 0},
                   float min = std::numeric_limits<float>::min(),
                   float max = std::numeric_limits<float>::max())
      : text_(text), value_(value), min_(min), max_(max) {}

  void Notification(int type) override;

  [[nodiscard]] Vector4 Value() const { return value_; }
  void SetValue(Vector4 value) { value_ = value; }

 private:
  std::string text_;
  Vector4 value_;
  float min_;
  float max_;
};
}  // namespace base_engine::editor
