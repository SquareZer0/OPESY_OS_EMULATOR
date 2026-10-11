#include "screens/ShutdownScreen.h"

#include <algorithm>

#include "imgui.h"
#include "screens/ScreenUtil.h"
#include "ui/Fonts.h"
#include "ui/Theme.h"
#include "ui/Wallpaper.h"

namespace {

constexpr double DURATION_SECONDS = 1.5;
constexpr float FONT_SIZE = 24.0f;

}  // namespace

void ShutdownScreen::render(double elapsed) {
    // The wallpaper fades to near-black while the text fades from dark to white.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 screenMax(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y);
    ImDrawList* background = ImGui::GetBackgroundDrawList();
    wallpaper::draw(background, viewport->Pos, screenMax);
    const float fade = static_cast<float>(std::min(1.0, elapsed / DURATION_SECONDS));
    background->AddRectFilled(viewport->Pos, screenMax, IM_COL32(20, 18, 30, static_cast<int>(255 * fade)));

    beginFullscreenWindow("##shutdown", ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    const ImVec4 dark = ImGui::ColorConvertU32ToFloat4(theme::TEXT);
    const ImVec4 textColor(dark.x + (1.0f - dark.x) * fade, dark.y + (1.0f - dark.y) * fade,
                           dark.z + (1.0f - dark.z) * fade, 1.0f);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.45f);
    ImGui::PushFont(fonts::semibold(), FONT_SIZE);
    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    textCentered("Shutting down...");
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::End();
}

bool ShutdownScreen::isDone(double elapsed) const {
    return elapsed >= DURATION_SECONDS;
}
