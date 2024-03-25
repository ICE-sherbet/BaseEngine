#include "SetupEditorImGui.h"


#include "imgui.h"

#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#if defined(__MOF__)

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);
#endif
namespace base_engine::editor {
#if defined(__MOF__)

void SetupEditorImGui::Setup(bool input_keyboard, bool input_controller) {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  if (input_keyboard)
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  if (input_controller)
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(g_pMainWindow->GetWindowHandle());
  ImGui_ImplDX11_Init(g_pGraphics->GetDevice(),
                      g_pGraphics->GetDeviceContext());
}

void SetupEditorImGui::Refresh() {
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
}

void SetupEditorImGui::RenderSetup() { ImGui::Render(); }

void SetupEditorImGui::RenderGui() {
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
}

void SetupEditorImGui::Cleanup() {
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

MofProcResult CMofImGuiProc::WindowProc(MofWindowHandle hWnd, MofUInt msg,
                                        MofProcParamW wpar,
                                        MofProcParamL lpar) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wpar, lpar)) return true;
  return CDefWindowProc::WindowProc(hWnd, msg, wpar, lpar);
}

#else
void SetupEditorImGui::Setup(bool input_keyboard, bool input_controller)
{
}

void SetupEditorImGui::Refresh()
{
}

void SetupEditorImGui::RenderSetup()
{
}

void SetupEditorImGui::RenderGui()
{
}

void SetupEditorImGui::Cleanup()
{
}

#endif
}  // namespace base_engine::editor
