#include "apps/Notepad.h"

Notepad::Notepad() : App("Notepad", "notepad.exe", icons::notepad, ImVec2(140.0f, 100.0f), ImVec2(480.0f, 360.0f)) {}

void Notepad::renderContents() {
    // PLACEHOLDER (Phase 2): multi-line text box (misc/cpp/imgui_stdlib.h lets
    // it edit a std::string), menu bar, and word count. See PLAN.md.
    ImGui::TextUnformatted("Notepad goes here.");
}
