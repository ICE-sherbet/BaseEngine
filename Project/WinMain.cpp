#include <GLFW/glfw3.h>

#include "Application.h"
#include "BaseEngineCore.h"
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
  base_engine::BaseEngineCore engine_;

  base_engine::ApplicationSpecification specification{};
  specification.width = 360;
  specification.height = 640;
  base_engine::Application application{specification};
  SetCurrentDirectory("Resource");
  application.Run();
  return 0;
}