#include "screens/SplashScreen.h"

#include <algorithm>

#include "imgui.h"
#include "screens/ScreenUtil.h"
#include "ui/Fonts.h"
#include "ui/Theme.h"
#include "ui/Wallpaper.h"

namespace {

constexpr double DURATION_SECONDS = 2.0;
constexpr float LOGO_FONT_SIZE = 52.0f;
constexpr ImVec2 CARD_SIZE(360.0f, 210.0f);
constexpr float CARD_ROUNDING = 26.0f;
constexpr float BAR_WIDTH = 220.0f;
constexpr float BAR_HEIGHT = 6.0f;

// Padded to the same width so the centered text doesn't shift as dots appear.
const char* const LOADING_FRAMES[] = {"Loading   ", "Loading.  ", "Loading.. ", "Loading..."};
constexpr double LOADING_FRAMES_PER_SECOND = 3.0;

}  // namespace

void SplashScreen::render(double elapsed) {
    // PLACEHOLDER (Phase 2): your own logo and animation can replace this.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 screenMax(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y);
    wallpaper::draw(ImGui::GetBackgroundDrawList(), viewport->Pos, screenMax);

    beginFullscreenWindow("##splash", ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // A glass card in the middle of the screen.
    const ImVec2 center = viewport->GetCenter();
    const ImVec2 cardMin(center.x - CARD_SIZE.x * 0.5f, center.y - CARD_SIZE.y * 0.5f);
    const ImVec2 cardMax(cardMin.x + CARD_SIZE.x, cardMin.y + CARD_SIZE.y);
    theme::drawGlassPanel(drawList, cardMin, cardMax, CARD_ROUNDING);

    ImGui::SetCursorScreenPos(ImVec2(cardMin.x, cardMin.y + 36.0f));
    ImGui::PushFont(fonts::semibold(), LOGO_FONT_SIZE);
    textCentered(theme::OS_NAME);
    ImGui::PopFont();

    // Loading bar: eases in and out so it doesn't fill at a robotic constant rate.
    const float t = static_cast<float>(std::min(1.0, elapsed / DURATION_SECONDS));
    const float progress = t * t * (3.0f - 2.0f * t);
    const ImVec2 barMin(center.x - BAR_WIDTH * 0.5f, cardMin.y + 132.0f);
    const ImVec2 barMax(barMin.x + BAR_WIDTH, barMin.y + BAR_HEIGHT);
    drawList->AddRectFilled(barMin, barMax, IM_COL32(255, 255, 255, 170), BAR_HEIGHT * 0.5f);
    if (progress > 0.0f) {
        drawList->AddRectFilled(barMin, ImVec2(barMin.x + BAR_WIDTH * progress, barMax.y), theme::ACCENT,
                                BAR_HEIGHT * 0.5f);
    }

    ImGui::SetCursorScreenPos(ImVec2(cardMin.x, barMax.y + 14.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(theme::TEXT_MUTED));
    textCentered(LOADING_FRAMES[static_cast<int>(elapsed * LOADING_FRAMES_PER_SECOND) % 4]);
    ImGui::PopStyleColor();

    ImGui::End();
}

bool SplashScreen::isDone(double elapsed) const {
    return elapsed >= DURATION_SECONDS;
}
