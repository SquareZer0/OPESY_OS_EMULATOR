#include "screens/BiosScreen.h"

#include <algorithm>

#include "imgui.h"
#include "screens/ScreenUtil.h"
#include "ui/Fonts.h"

namespace {

constexpr double DURATION_SECONDS = 4.5;
constexpr double LINE_INTERVAL_SECONDS = 0.35;  // delay before each new line appears
constexpr float FONT_SIZE = 20.0f;

// PLACEHOLDER (Phase 2): replace with your own PC's specs and add the memory
// test count-up and drive detection described in PLAN.md.
const char* const LINES[] = {
    "pe$OS BIOS v0.1",
    "",
    "CPU: <your CPU here>",
    "Memory Test: 16384 MB OK",
    "",
    "Detecting drives...",
    "  Primary Master: <your drive here>",
    "",
    "Booting pe$OS...",
};
constexpr int LINE_COUNT = sizeof(LINES) / sizeof(LINES[0]);

}  // namespace

void BiosScreen::render(double elapsed) {
    beginFullscreenWindow("##bios", ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushFont(fonts::mono(), FONT_SIZE);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));

    const int visibleLines = std::min(LINE_COUNT, static_cast<int>(elapsed / LINE_INTERVAL_SECONDS) + 1);
    for (int i = 0; i < visibleLines; ++i) {
        ImGui::TextUnformatted(LINES[i]);
    }

    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::End();
}

bool BiosScreen::isDone(double elapsed) const {
    return elapsed >= DURATION_SECONDS;
}
