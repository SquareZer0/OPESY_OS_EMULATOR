#include "screens/ShutdownScreen.h"

#include "imgui.h"
#include "screens/ScreenUtil.h"

namespace {

constexpr double DURATION_SECONDS = 1.5;
constexpr float FONT_SIZE = 28.0f;

}  // namespace

void ShutdownScreen::render() {
    beginFullscreenWindow("##shutdown", ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.45f);
    ImGui::PushFont(nullptr, FONT_SIZE);
    textCentered("Shutting down...");
    ImGui::PopFont();
    ImGui::End();
}

bool ShutdownScreen::isDone(double elapsed) const {
    return elapsed >= DURATION_SECONDS;
}
