#include "screens/SplashScreen.h"

#include <algorithm>

#include "imgui.h"
#include "screens/ScreenUtil.h"

namespace {

constexpr double DURATION_SECONDS = 3.0;
constexpr float TITLE_FONT_SIZE = 48.0f;

// Padded to the same width so the centered text doesn't shift as dots appear.
const char* const LOADING_FRAMES[] = {"Loading   ", "Loading.  ", "Loading.. ", "Loading..."};
constexpr double LOADING_FRAMES_PER_SECOND = 3.0;

}  // namespace

void SplashScreen::render(double elapsed) {
    // PLACEHOLDER (Phase 2): replace with your own logo and animation design.
    beginFullscreenWindow("##splash", ImVec4(0.06f, 0.09f, 0.16f, 1.0f));
    const ImVec2 size = ImGui::GetWindowSize();

    ImGui::SetCursorPosY(size.y * 0.40f);
    ImGui::PushFont(nullptr, TITLE_FONT_SIZE);
    textCentered("MockOS");
    ImGui::PopFont();

    ImGui::Spacing();
    const float barWidth = size.x * 0.3f;
    ImGui::SetCursorPosX((size.x - barWidth) * 0.5f);
    const float progress = static_cast<float>(std::min(1.0, elapsed / DURATION_SECONDS));
    ImGui::ProgressBar(progress, ImVec2(barWidth, 0.0f), "");

    textCentered(LOADING_FRAMES[static_cast<int>(elapsed * LOADING_FRAMES_PER_SECOND) % 4]);
    ImGui::End();
}

bool SplashScreen::isDone(double elapsed) const {
    return elapsed >= DURATION_SECONDS;
}
