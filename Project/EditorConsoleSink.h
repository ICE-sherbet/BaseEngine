// @EditorConsoleSink.h
// @brief
// @author ICE
// @date 2023/05/29
//
// @details

#pragma once
#include <spdlog/sinks/base_sink.h>

#include "ConsoleMessage.h"
#include "ConsolePanel.h"

namespace base_engine::editor {

class EditorConsoleSink : public spdlog::sinks::base_sink<std::mutex> {
 public:
  explicit EditorConsoleSink(const uint32_t buffer_capacity)
      : message_buffer_capacity_(buffer_capacity),
        message_buffer_(buffer_capacity) {}

  ~EditorConsoleSink() override = default;

  EditorConsoleSink(const EditorConsoleSink& other) = delete;
  EditorConsoleSink& operator=(const EditorConsoleSink& other) = delete;

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
    const std::string long_message = fmt::to_string(formatted);
    std::string short_message = long_message;

    if (short_message.length() > 100) {
      if (const size_t space_pos = short_message.find_first_of(' ', 100);
          space_pos != std::string::npos)
        short_message.replace(space_pos, short_message.length() - 1, "...");
    }

    message_buffer_[message_count_++] =
        ConsoleMessage{short_message, long_message, GetMessageFlags(msg.level),
                       std::chrono::system_clock::to_time_t(msg.time)};

    if (message_count_ == message_buffer_capacity_) flush_();
  }

  void flush_() override {
    for (const auto& message : message_buffer_)
      ConsolePanel::PushMessage(message);

    message_count_ = 0;
  }

 private:
  static ConsoleMessageType GetMessageFlags(
      const spdlog::level::level_enum level) {
    ConsoleMessageType flags;
    switch (level) {
      case spdlog::level::trace:
      case spdlog::level::debug:
      case spdlog::level::info: {
        flags = ConsoleMessageType::kInfo;
        break;
      }
      case spdlog::level::warn: {
        flags = ConsoleMessageType::kWarning;
        break;
      }
      case spdlog::level::err:
      case spdlog::level::critical: {
        flags = ConsoleMessageType::kError;
        break;
      }
      case spdlog::level::off:
      case spdlog::level::n_levels:
      default:
        flags = ConsoleMessageType::kInfo;

    }

    return flags;
  }

 private:
  uint32_t message_buffer_capacity_;
  std::vector<ConsoleMessage> message_buffer_;
  uint32_t message_count_ = 0;
};
}  // namespace base_engine::editor
