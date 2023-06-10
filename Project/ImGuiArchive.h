// @ImGuiArchive.h
// @brief
// @author ICE
// @date 2023/06/09
//
// @details

#pragma once
#include "FrozenHelper.h"
#include "frozen.h"
#include "imgui.h"
#include "Vector3.h"

namespace base_engine::editor {
class ImGuiArchive : public frozen::InputArchive<ImGuiArchive> {
 public:
  explicit ImGuiArchive() : InputArchive(this) {}

  template <class T>
  void Draw(const char* label, T& value) {}

 private:
};
template <class T>
void FROZEN_LOAD_FUNCTION_NAME(ImGuiArchive& ar, T& t) {
  ar.Draw(t);
}
template <class T>
void FROZEN_LOAD_FUNCTION_NAME(ImGuiArchive& ar, frozen::NameValuePair<T>& t) {
  ar.Draw(t);
}
}  // namespace base_engine::editor
