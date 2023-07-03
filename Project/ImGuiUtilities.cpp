#include "ImGuiUtilities.h"

#include <format>
#include <fstream>

namespace base_engine::editor::ui {
static int context_id = 0;
static uint32_t s_counter = 0;
static char s_id_buffer[16 + 2 + 1] = "##";
static char s_label_id_buffer[1024 + 1];

const char* GenerateID() {
  snprintf(s_id_buffer + 2, 16, "%u", s_counter++);
  return s_id_buffer;
}

const char* GenerateLabelID(std::string_view label) {
  *std::format_to_n(s_label_id_buffer, std::size(s_label_id_buffer), "{}##{}",
                    label, s_counter++)
       .out = 0;
  return s_label_id_buffer;
}
void PushID()
{
	ImGui::PushID(context_id++);
  s_counter = 0;
}

void PopID() {
  ImGui::PopID();
  context_id--;
}
}  // namespace base_engine::editor::ui
