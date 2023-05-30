// @Algorithm.h
// @brief
// @author ICE
// @date 2023/03/16
//
// @details

#pragma once
#include <algorithm>
#include <functional>

namespace becs {
struct std_sort {
  template <typename It, typename Compare = std::less<>, typename... Args>
  void operator()(It first, It last, Compare compare = Compare{},
                  Args &&...args) const {
    std::sort(std::forward<Args>(args)..., std::move(first), std::move(last),
              std::move(compare));
  }
};
}  // namespace becs