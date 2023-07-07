#include "MenuBarPanel.h"
#include "ImGuiUtilities.h"
#include <imgui.h>

#include "CSharpScriptEngine.h"

namespace base_engine::editor
{
void MenuBarPanel::OnImGuiRender()
{
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      if (ImGui::MenuItem(ui::GenerateLabelID("Reload C# Assembly")))
      {
	      CSharpScriptEngine::GetInstance()->ReloadMonoAssembly();
      };
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}

void MenuBarPanel::Initialize(const Ref<Scene>& context)
{
  SetSceneContext(context);
}

void MenuBarPanel::SetSceneContext(const Ref<Scene>& context)
{
  scene_context_ = context;
}
}
