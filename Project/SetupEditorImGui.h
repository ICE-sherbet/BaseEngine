// @SetupEditorImGui.h
// @brief
// @author ICE
// @date 2023/05/19
//
// @details

#pragma once
#include <Framework/Window/Windows/DefWindowProc.h>

namespace base_engine::editor {
class SetupEditorImGui {
  SetupEditorImGui() = delete;
  ~SetupEditorImGui() = delete;

 public:
  static void Setup(bool input_keyboard = true, bool input_controller = true);

  static void Refresh();

  static void RenderSetup();

  static void RenderGui();

  static void Cleanup();
};
class CMofImGuiProc : public Mof::CDefWindowProc {
 public:
  virtual MofProcResult WindowProc(MofWindowHandle hWnd, MofUInt msg,
                                   MofProcParamW wpar,
                                   MofProcParamL lpar) override;
};
}  // namespace base_engine::editor
