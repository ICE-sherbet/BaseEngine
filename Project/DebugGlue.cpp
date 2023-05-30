#include "DebugGlue.h"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>

#include "CSharpScriptEngine.h"
#include "Log.h"

namespace base_engine::glue::internal_calls {
void DebugLog(MonoString* message) {
  MonoObject* stackTraceInstance =

      CSharpScriptEngine::GetInstance()->CreateManagedObject(
          CSharpScriptEngine::GetInstance()->GetManagedClassByName(
              "System.Diagnostics.StackTrace"));
  MonoObject* exception = nullptr;
  MonoString* stackTraceString =
      mono_object_to_string(stackTraceInstance, &exception);
  MonoError error;
  char* trace = mono_string_to_utf8_checked(stackTraceString, &error);

  std::string result(trace);
  auto log_message = mono_string_to_utf8(message);
  {
    const auto info =
        std::format("C# Log:{0}\nStackTrace:{1}", log_message, trace);
    BE_CORE_INFO(info);
  }
  mono_free(log_message);
  mono_free(trace);
}
}  // namespace base_engine::glue::internal_calls