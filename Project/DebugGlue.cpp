#include "DebugGlue.h"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>

#include "CSharpScriptEngine.h"
#include "Log.h"

namespace base_engine::glue::internal_calls {
void DebugLog(MonoString* message) {

  MonoError error;

  auto log_message = mono_string_to_utf8(message);
  {
    const auto info =
        std::format("C# Log:{0}", log_message);
    BE_CORE_INFO(info);
  }
  mono_free(log_message);

}
}  // namespace base_engine::glue::internal_calls