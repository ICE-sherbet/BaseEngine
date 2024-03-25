#include "InputGlue.h"


#include "BaseEngineInput.h"
#include "imgui.h"

#if defined(__MOF__)
namespace base_engine::glue::internal_calls {
bool InputGetKeyDown(const int16_t key) {
  if (!BaseEngineInput::can_input_) return false;
  const auto code = MapVirtualKeyA(key, MAPVK_VK_TO_VSC_EX);
  return g_pInput->IsKeyPush(code);
}

bool InputGetKey(const int16_t key) {
  if (!BaseEngineInput::can_input_) return false;

  const auto code = MapVirtualKeyA(key, MAPVK_VK_TO_VSC_EX);
  return g_pInput->IsKeyHold(code);
}

bool InputGetKeyUp(const int16_t key) {
  if (!BaseEngineInput::can_input_) return false;

  const auto code = MapVirtualKeyA(key, MAPVK_VK_TO_VSC_EX);
  return g_pInput->IsKeyPull(code);
}
}  // namespace base_engine::glue::internal_calls
#endif

namespace base_engine::glue::internal_calls {

bool InputGetKeyDown(const int16_t key) { return false; }

bool InputGetKey(const int16_t key) { return false; }

bool InputGetKeyUp(const int16_t key) { return false; }
}  // namespace base_engine::glue::internal_calls