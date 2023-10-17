#include "GameApp.h"
#include "GameWindow.h"
#include "IWindow.h"
#include "Log.h"
#include "SetupEditorImGui.h"
#include "resource1.h"
#ifdef UNICODE
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPTSTR lpCmdLine, int nCmdShow)
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPTSTR lpCmdLine, int nCmdShow)
#endif
{
  base_engine::Log::Init();
  auto window = base_engine::IWindow::Create();
  window->Init();
  while (true) {
    window->Update();
  }
  //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  Mof::LPFramework pFrame = new Mof::CDX11GameFramework();
  Mof::WINDOWSGAMEFRAMEWORKINITIALIZEINFO Info;
  Info.pApplication = new CGameApp();
  Info.WindowCreateInfo.pProc = new base_engine::editor::CMofImGuiProc();
  Info.WindowCreateInfo.Width = window::kWidth * 0.75f;
  Info.WindowCreateInfo.Height = window::kHeight * 0.75f;
  Info.WindowCreateInfo.Title = "BaseEngine";
  //  ShowCursor(false);
  auto icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
  Info.WindowCreateInfo.hIcon = icon;
  ;
  // Info.GraphicsCreateInfo.bWindowed = true;
  pFrame->Initialize(&Info);
  pFrame->Run();
  MOF_SAFE_DELETE(pFrame);
  return 0;
}