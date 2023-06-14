// @Log.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details
#pragma once

#include <spdlog/logger.h>

#include <map>
#include <memory>
#include <string>
#define BE_ASSERT_MESSAGE_BOX !DEBUG

#if BE_ASSERT_MESSAGE_BOX
#include <Windows.h>
#endif

namespace base_engine {

class Log {
 public:
  enum class Type : uint8_t { kCore = 0, kClient = 1 };
  enum class Level : uint8_t { kTrace = 0, kInfo, kWarn, kError, kFatal };
  struct TagDetails {
    bool Enabled = true;
    Level LevelFilter = Level::kTrace;
  };

 public:
  static void Init();
  static void Shutdown();

  inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() {
    return s_CoreLogger;
  }
  inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {
    return s_ClientLogger;
  }
  inline static std::shared_ptr<spdlog::logger>& GetEditorConsoleLogger() {
    return s_EditorConsoleLogger;
  }

  static bool HasTag(const std::string& tag) {
    return s_EnabledTags.contains(tag);
  }
  static std::map<std::string, TagDetails>& EnabledTags() {
    return s_EnabledTags;
  }

  template <typename... Args>
  static void PrintMessage(Log::Type type, Log::Level level,
                           std::string_view tag, Args&&... args);

  template <typename... Args>
  static void PrintAssertMessage(Log::Type type, std::string_view prefix,
                                 Args&&... args);

 public:
  static const char* LevelToString(const Level level) {
    switch (level) {
      case Level::kTrace:
        return "Trace";
      case Level::kInfo:
        return "Info";
      case Level::kWarn:
        return "Warn";
      case Level::kError:
        return "Error";
      case Level::kFatal:
        return "Fatal";
    }
    return "";
  }
  static Level LevelFromString(std::string_view string) {
    if (string == "Trace") return Level::kTrace;
    if (string == "Info") return Level::kInfo;
    if (string == "Warn") return Level::kWarn;
    if (string == "Error") return Level::kError;
    if (string == "Fatal") return Level::kFatal;

    return Level::kTrace;
  }

 private:
  static std::shared_ptr<spdlog::logger> s_CoreLogger;
  static std::shared_ptr<spdlog::logger> s_ClientLogger;
  static std::shared_ptr<spdlog::logger> s_EditorConsoleLogger;

  inline static std::map<std::string, TagDetails> s_EnabledTags;
};

}  // namespace base_engine

// Core logging
#define BE_CORE_TRACE_TAG(tag, ...)                                        \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kCore,        \
                                   ::base_engine::Log::Level::kTrace, tag, \
                                   __VA_ARGS__)
#define BE_CORE_INFO_TAG(tag, ...)                                        \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kCore,       \
                                   ::base_engine::Log::Level::kInfo, tag, \
                                   __VA_ARGS__)
#define BE_CORE_WARN_TAG(tag, ...)                                        \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kCore,       \
                                   ::base_engine::Log::Level::kWarn, tag, \
                                   __VA_ARGS__)
#define BE_CORE_ERROR_TAG(tag, ...)                                        \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kCore,        \
                                   ::base_engine::Log::Level::kError, tag, \
                                   __VA_ARGS__)
#define BE_CORE_FATAL_TAG(tag, ...)                                        \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kCore,        \
                                   ::base_engine::Log::Level::kFatal, tag, \
                                   __VA_ARGS__)

// Client logging
#define BE_TRACE_TAG(tag, ...)                                             \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kClient,      \
                                   ::base_engine::Log::Level::kTrace, tag, \
                                   __VA_ARGS__)
#define BE_INFO_TAG(tag, ...)                                             \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kClient,     \
                                   ::base_engine::Log::Level::kInfo, tag, \
                                   __VA_ARGS__)
#define BE_WARN_TAG(tag, ...)                                             \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kClient,     \
                                   ::base_engine::Log::Level::kWarn, tag, \
                                   __VA_ARGS__)
#define BE_ERROR_TAG(tag, ...)                                             \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kClient,      \
                                   ::base_engine::Log::Level::kError, tag, \
                                   __VA_ARGS__)
#define BE_FATAL_TAG(tag, ...)                                             \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kClient,      \
                                   ::base_engine::Log::Level::kFatal, tag, \
                                   __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core Logging
#define BE_CORE_TRACE(...)                                                \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kCore,       \
                                   ::base_engine::Log::Level::kTrace, "", \
                                   __VA_ARGS__)
#define BE_CORE_INFO(...)                                                \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kCore,      \
                                   ::base_engine::Log::Level::kInfo, "", \
                                   __VA_ARGS__)
#define BE_CORE_WARN(...)                                                \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kCore,      \
                                   ::base_engine::Log::Level::kWarn, "", \
                                   __VA_ARGS__)
#define BE_CORE_ERROR(...)                                                \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kCore,       \
                                   ::base_engine::Log::Level::kError, "", \
                                   __VA_ARGS__);                          \
                                   __debugbreak();
#define BE_CORE_FATAL(...)                                                \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kCore,       \
                                   ::base_engine::Log::Level::kFatal, "", \
                                   __VA_ARGS__)

// Client Logging
#define BE_TRACE(...)                                                     \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kClient,     \
                                   ::base_engine::Log::Level::kTrace, "", \
                                   __VA_ARGS__)
#define BE_INFO(...)                                                     \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kClient,    \
                                   ::base_engine::Log::Level::kInfo, "", \
                                   __VA_ARGS__)
#define BE_WARN(...)                                                     \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kClient,    \
                                   ::base_engine::Log::Level::kWarn, "", \
                                   __VA_ARGS__)
#define BE_ERROR(...)                                                     \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kClient,     \
                                   ::base_engine::Log::Level::kError, "", \
                                   __VA_ARGS__)
#define BE_FATAL(...)                                                     \
  ::base_engine::Log::PrintMessage(::base_engine::Log::Type::kClient,     \
                                   ::base_engine::Log::Level::kFatal, "", \
                                   __VA_ARGS__)

// Editor Console Logging Macros
#define BE_CONSOLE_LOG_TRACE(...) \
  base_engine::Log::GetEditorConsoleLogger()->trace(__VA_ARGS__)
#define BE_CONSOLE_LOG_INFO(...) \
  base_engine::Log::GetEditorConsoleLogger()->info(__VA_ARGS__)
#define BE_CONSOLE_LOG_WARN(...) \
  base_engine::Log::GetEditorConsoleLogger()->warn(__VA_ARGS__)
#define BE_CONSOLE_LOG_ERROR(...) \
  base_engine::Log::GetEditorConsoleLogger()->error(__VA_ARGS__)
#define BE_CONSOLE_LOG_FATAL(...) \
  base_engine::Log::GetEditorConsoleLogger()->critical(__VA_ARGS__)

namespace base_engine {

template <typename... Args>
void Log::PrintMessage(Log::Type type, Log::Level level, std::string_view tag,
                       Args&&... args) {
  auto detail = s_EnabledTags[std::string(tag)];
  if (detail.Enabled && detail.LevelFilter <= level) {
    auto logger = (type == Type::kCore) ? GetCoreLogger() : GetClientLogger();
    std::string logString = tag.empty() ? "{0}{1}" : "[{0}] {1}";
    switch (level) {
      case Level::kTrace:
        logger->trace(logString, tag, fmt::format(std::forward<Args>(args)...));
        break;
      case Level::kInfo:
        logger->info(logString, tag, fmt::format(std::forward<Args>(args)...));
        break;
      case Level::kWarn:
        logger->warn(logString, tag, fmt::format(std::forward<Args>(args)...));
        break;
      case Level::kError:
        logger->error(logString, tag, fmt::format(std::forward<Args>(args)...));
        break;
      case Level::kFatal:
        logger->critical(logString, tag,
                         fmt::format(std::forward<Args>(args)...));
        break;
    }
  }
}

template <typename... Args>
void Log::PrintAssertMessage(Log::Type type, std::string_view prefix,
                             Args&&... args) {
  auto logger = (type == Type::kCore) ? GetCoreLogger() : GetClientLogger();
  logger->error("{0}: {1}", prefix, fmt::format(std::forward<Args>(args)...));

#if BE_ASSERT_MESSAGE_BOX
  std::string message = fmt::format(std::forward<Args>(args)...);
  MessageBoxA(nullptr, message.c_str(), "base_engine Assert",
              MB_OK | MB_ICONERROR);
#endif
}

template <>
inline void Log::PrintAssertMessage(const Log::Type type,
                                    std::string_view prefix) {
  const auto logger =
      (type == Type::kCore) ? GetCoreLogger() : GetClientLogger();
  logger->error("{0}", prefix);
#if BE_ASSERT_MESSAGE_BOX
  MessageBoxA(nullptr, "No message :(", "base_engine Assert",
              MB_OK | MB_ICONERROR);
#endif
}
}  // namespace base_engine
