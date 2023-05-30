// @ConsolePanel.h
// @brief
// @author ICE
// @date 2023/05/29
// 
// @details

#pragma once
#include "ConsoleMessage.h"
#include "EditorPanel.h"

namespace base_engine::editor {

class ConsolePanel : public EditorPanel {
public:
	ConsolePanel();
	void OnImGuiRender() override;
	static void PushMessage(const ConsoleMessage& message);

private:
	std::vector<ConsoleMessage> messages_buffer_;




	static ConsolePanel* instance_;
	
};
}  // namespace base_engine::editor