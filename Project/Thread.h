// @Thread.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include <string>
#include <thread>

namespace base_engine {
class Thread {
 public:
  Thread(const std::string& name);

  template <typename Fn, typename... Args>
  void Dispatch(Fn&& func, Args&&... args) {
    thread_ = std::thread(func, std::forward<Args>(args)...);
    SetName(name_);
  }

  void SetName(const std::string& name);

  void Join();

private:
  std::string name_;
  std::thread thread_;
};
}  // namespace base_engine
