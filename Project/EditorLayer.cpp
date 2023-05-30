#include "EditorLayer.h"

#include "SetupEditorImGui.h"
#include "imgui.h"

namespace base_engine::editor {
void EditorLayer::Initialize(const Ref<Scene>& scene) {
  SetupEditorImGui::Setup();
  panel_manager_ = std::make_unique<EditorPanelManager>(game_);
  panel_manager_->SetSceneContext(scene);
  panel_manager_->Initialize();
}

void EditorLayer::OnUpdate() {
  SetupEditorImGui::Refresh();
  panel_manager_->OnUpdate();
}

void EditorLayer::OnRender() {
  panel_manager_->OnRender();
  SetupEditorImGui::RenderSetup();
  SetupEditorImGui::RenderGui();
}

void EditorLayer::SetSceneContext(const Ref<Scene>& scene) {
  panel_manager_->SetSceneContext(scene);
}

EditorLayer::~EditorLayer() { SetupEditorImGui::Cleanup(); }
}  // namespace base_engine::editor
