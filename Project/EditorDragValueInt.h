// @EditorDragValueInt.h
// @brief
// @author ICE
// @date 2023/07/04
// 
// @details

#pragma once
#include <string>

#include "EditorControl.h"

namespace base_engine::editor
{
class EditorDragValueInt : public EditorControl {
public:
  EditorDragValueInt(const std::string& text = "", float value = 0,
                       float min = std::numeric_limits<float>::min(),
                       float max = std::numeric_limits<float>::max())
      : text_(text), value_(value), min_(min), max_(max) {}


  void Notification(int type) override;
  [[nodiscard]] float Value() const { return value_; }
  void SetValue(float value) { value_ = value; }

  void Setup(int64_t min, int64_t max, int64_t step);

private:
  std::string text_;
  float value_;
  float min_;
  float max_;
};
}
