// @ConsoleMessage.h
// @brief
// @author ICE
// @date 2023/05/29
//
// @details

#pragma once
#include <cstdint>
#include <ctime>
#include <string>

namespace base_engine::editor {
enum class ConsoleMessageType { kInfo, kWarning, kError };

struct ConsoleMessage {
  std::string short_message;
  std::string long_message;
  ConsoleMessageType type;

  time_t timestamp;
};
}  // namespace base_engine::editor
