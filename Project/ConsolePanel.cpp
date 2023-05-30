#include "ConsolePanel.h"
namespace base_engine::editor {

ConsolePanel* ConsolePanel::instance_ = nullptr;

ConsolePanel::ConsolePanel()
{
	instance_ = this;
}

void ConsolePanel::OnImGuiRender() {}

void ConsolePanel::PushMessage(const ConsoleMessage& message) {}
}  // namespace base_engine::editor
