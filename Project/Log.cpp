#include "Log.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>

#define BE_HAS_CONSOLE !BE_DIST

namespace base_engine {

std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
std::shared_ptr<spdlog::logger> Log::s_EditorConsoleLogger;

void Log::Init() {
  if (const std::string logs_directory = "logs";
      !std::filesystem::exists(logs_directory))
    std::filesystem::create_directories(logs_directory);

  std::vector<spdlog::sink_ptr> base_engine_sinks = {
    std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/base_engine.log",
                                                        true),
#if BE_HAS_CONSOLE
    std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
#endif
  };

  std::vector<spdlog::sink_ptr> app_sinks = {
    std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/APP.log", true),
#if BE_HAS_CONSOLE
    std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
#endif
  };

  std::vector<spdlog::sink_ptr> editor_console_sinks = {
    std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/APP.log", true),
#if BE_HAS_CONSOLE
  // TODO エディタ用コンソールにログ出力

#endif
  };

  base_engine_sinks[0]->set_pattern("[%T] [%l] %n: %v");
  app_sinks[0]->set_pattern("[%T] [%l] %n: %v");

#if BE_HAS_CONSOLE
  base_engine_sinks[1]->set_pattern("%^[%T] %n: %v%$");
  app_sinks[1]->set_pattern("%^[%T] %n: %v%$");
  for (const auto sink : editor_console_sinks) sink->set_pattern("%^%v%$");
#endif

  s_CoreLogger = std::make_shared<spdlog::logger>(
      "base_engine", base_engine_sinks.begin(), base_engine_sinks.end());
  s_CoreLogger->set_level(spdlog::level::trace);

  s_ClientLogger = std::make_shared<spdlog::logger>("APP", app_sinks.begin(),
                                                    app_sinks.end());
  s_ClientLogger->set_level(spdlog::level::trace);

  s_EditorConsoleLogger = std::make_shared<spdlog::logger>(
      "Console", editor_console_sinks.begin(), editor_console_sinks.end());
  s_EditorConsoleLogger->set_level(spdlog::level::trace);
}

void Log::Shutdown() {
  s_EditorConsoleLogger.reset();
  s_ClientLogger.reset();
  s_CoreLogger.reset();
  spdlog::drop_all();
}

}  // namespace base_engine
