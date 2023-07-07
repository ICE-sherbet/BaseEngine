#include "EditorPanelManager.h"

#include "HierarchyPanel.h"
#include "imgui.h"
#include "InspectorPanel.h"
#include "ToolbarPanel.h"
#include "AssetsBrowserPanel.h"
#include "MenuBarPanel.h"

namespace base_engine::editor {
void EditorPanelManager::Initialize() {
  panels_.emplace_back(std::make_shared<HierarchyPanel>());
  panels_.emplace_back(std::make_shared<InspectorPanel>());
  panels_.emplace_back(std::make_shared<ToolbarPanel>(this));
  panels_.emplace_back(std::make_shared<AssetsBrowserPanel>());
  panels_.emplace_back(std::make_shared<MenuBarPanel>());

  for (const auto& editor_panel : panels_) {
    editor_panel->Initialize(scene_context_);
  }
}

void EditorPanelManager::OnUpdate() {
  bool open = true;
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_MenuBar;
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
  {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |=
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }

  ImGui::Begin("DockSpace Demo", &open, window_flags);

  ImGui::PopStyleVar(2);
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  ImGui::End();
}

void EditorPanelManager::OnRender()
{
  for (const auto& editor_panel : panels_) {
    editor_panel->OnImGuiRender();
  }
}

void EditorPanelManager::SetSceneContext(const Ref<Scene>& scene) {
  scene_context_ = scene;
  for (const auto& panel : panels_) {
    panel->SetSceneContext(scene);
  }
}
}  // namespace base_engine::editor
