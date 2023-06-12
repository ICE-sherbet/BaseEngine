#include "AssetsBrowserPanel.h"

#include "imgui.h"

namespace base_engine::editor
{
void AssetsBrowserPanel::OnImGuiRender()
{
  ImGui::Begin("Assets Browser");

  ImGui::End();

}

void AssetsBrowserPanel::Initialize(const Ref<Scene>& context)
{}

void AssetsBrowserPanel::SetSceneContext(const Ref<Scene>& context)
{}
}
