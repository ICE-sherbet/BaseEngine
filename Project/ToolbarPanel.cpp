#include "ToolbarPanel.h"

#include <Mof.h>

#include "BaseEngineCore.h"
#include "BaseEngineInput.h"
#include "CSharpScriptEngine.h"
#include "IBaseEngineRender.h"
#include "imgui.h"
#include "imgui/backends/imgui_impl_dx11.h"
namespace base_engine::editor {
void ToolbarPanel::OnImGuiRender() {
  ImGui::Begin("##viewport_central_toolbar", 0, ImGuiWindowFlags_NoDecoration);
  if (is_play_) {
    if (ImGui::Button("Stop")) {
      is_play_ = false;
      OnSceneStop();
    }
  } else {
    if (ImGui::Button("Play")) {
      is_play_ = true;
      OnScenePlay();
    };
  }
  ImGui::End();
  ImGui::Begin("Viewport");
  const auto window_size = ImGui::GetWindowSize();

	const auto resource = BASE_ENGINE(Render)->GetTargetTexture();
  texture_ = resource->GetTexture();
  ImGui::Image(texture_, ImVec2{window_size.x, window_size.x * (9.0f / 16.0f)});

  BaseEngineInput::can_input_ = ImGui::IsWindowFocused();
  ImGui::End();
}

void ToolbarPanel::SetSceneContext(const Ref<Scene>& context) {
  if (is_play_) return;
  context_scene_ = context;
}

void ToolbarPanel::Initialize(const Ref<Scene>& context) {
  context_scene_ = context;
}

void ToolbarPanel::OnScenePlay() {
  runtime_scene_ = Ref<Scene>::Create();
  runtime_scene_->OnInit();
  context_scene_->CopyTo(runtime_scene_.Raw());
  CSharpScriptEngine::GetInstance()->SetScene(runtime_scene_);
  panel_manager_->SetSceneContext(runtime_scene_);
  runtime_scene_->OnRuntimeStart();
  panel_manager_->game_->SetScene(runtime_scene_);
}

void ToolbarPanel::OnSceneStop() {
  runtime_scene_->OnRuntimeStop();

  CSharpScriptEngine::GetInstance()->SetScene(context_scene_);
  panel_manager_->SetSceneContext(context_scene_);
  panel_manager_->game_->SetScene(context_scene_);
}
}  // namespace base_engine::editor
