#pragma once

struct ImFont;

// The program's fonts. Change sizes with ImGui::PushFont(font, size).
namespace fonts {

// Loads every font below. Call once, right after ImGui::CreateContext().
void load();

ImFont* regular();   // Poppins Regular, the default for all text
ImFont* semibold();  // Poppins SemiBold, for headings
ImFont* mono();      // monospace, for the BIOS screen

}  // namespace fonts
