#include "screens/Taskbar.h"

#include <ctime>

#include "apps/AppManager.h"
#include "imgui.h"
#include "imgui_internal.h"  // BringWindowToDisplayFront, GetCurrentWindow

namespace {

constexpr float POWER_BUTTON_WIDTH = 60.0f;

}  // namespace

bool Taskbar::render(AppManager& apps) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - HEIGHT));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, HEIGHT));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##taskbar", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    ImGui::PopStyleVar();

    // ImGui draws the most recently clicked window on top, so a dragged app
    // window could cover the taskbar. Raise the taskbar every frame, except
    // while a popup (like the PWR dialog) is open, since that must stay on top.
    if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopup)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    }

    const float buttonHeight = HEIGHT - 2.0f * ImGui::GetStyle().WindowPadding.y;
    const bool powerClicked = ImGui::Button("PWR", ImVec2(POWER_BUTTON_WIDTH, buttonHeight));

    // PLACEHOLDER (Phase 2): icon buttons instead of text labels, and a
    // clearer running-apps indicator. For now, open apps get a brighter button.
    for (const auto& app : apps.list()) {
        ImGui::SameLine();
        const bool running = app->isOpen();
        if (running) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }
        if (ImGui::Button(app->name().c_str(), ImVec2(0.0f, buttonHeight))) {
            apps.onTaskbarClick(*app);
        }
        if (running) {
            ImGui::PopStyleColor();
        }
    }

    renderClock();
    ImGui::End();
    return powerClicked;
}

void Taskbar::renderClock() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    char text[32];
    std::strftime(text, sizeof(text), "%m/%d/%Y  %I:%M %p", &local);

    // Right-aligned and vertically centered, recomputed every frame so it
    // stays in the corner when the window is resized.
    const ImGuiStyle& style = ImGui::GetStyle();
    const float textWidth = ImGui::CalcTextSize(text).x;
    ImGui::SameLine(ImGui::GetWindowWidth() - textWidth - style.WindowPadding.x);
    ImGui::SetCursorPosY((HEIGHT - ImGui::GetTextLineHeight()) * 0.5f);
    ImGui::TextUnformatted(text);
}
