#pragma once
#include <vector>

#include "Vector2.h"
template <typename T>
concept SendablePower = requires(T& t) {
  t.CanSending();
};
class ISendablePower {
 public:
  virtual ~ISendablePower() = default;
  virtual bool CanSending() = 0;
  virtual const ISendablePower* GetPrevious() = 0;
  virtual void SetPrevious(ISendablePower* previous) = 0;

  [[nodiscard]] virtual base_engine::Vector2 GetPosition() const = 0;
};
