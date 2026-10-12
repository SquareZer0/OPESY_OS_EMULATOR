#include "screens/Desktop.h"

#include <cstdio>

#include "imgui.h"
#include "ui/Fonts.h"
#include "ui/Icons.h"
#include "ui/Theme.h"
#include "ui/Wallpaper.h"

namespace {

const char* const POWER_POPUP_ID = "Shut Down";
const ImVec2 POWER_BUTTON_SIZE(130.0f, 0.0f);
constexpr float POWER_ICON_SIZE = 34.0f;

}  // namespace

void Desktop::render() {
    // Drawn on the background draw list so it sits behind every window, and
    // sized from the viewport every frame so it follows window resizes.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    wallpaper::draw(ImGui::GetBackgroundDrawList(), viewport->Pos,
                    ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y));

    apps_.render();
    if (topBar_.render(apps_)) {
        requestPowerDialog();
    }
    taskbar_.render(apps_);
    renderPowerDialog();

#ifndef NDEBUG
    // F12 shows Dear ImGui's demo window: every widget, with the code for it.
    if (ImGui::IsKeyPressed(ImGuiKey_F12, false)) {
        showImGuiDemo_ = !showImGuiDemo_;
    }
    if (showImGuiDemo_) {
        ImGui::ShowDemoWindow(&showImGuiDemo_);
    }
#endif
}

void Desktop::renderPowerDialog() {
    // OpenPopup() has to be called from the same place as BeginPopupModal(),
    // so the power button and the X button only set a flag and the popup opens here.
    if (powerDialogRequested_) {
        ImGui::OpenPopup(POWER_POPUP_ID);
        powerDialogRequested_ = false;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 20.0f));
    const bool open = ImGui::BeginPopupModal(
        POWER_POPUP_ID, nullptr,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
    ImGui::PopStyleVar();
    if (!open) {
        return;
    }

    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    theme::drawShadow(drawList, windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                      ImGui::GetStyle().PopupRounding);

    // Power icon centered above the text.
    const ImVec2 iconTopLeft = ImGui::GetCursorScreenPos();
    const float contentWidth = POWER_BUTTON_SIZE.x * 2.0f + ImGui::GetStyle().ItemSpacing.x;
    ImGui::Dummy(ImVec2(contentWidth, POWER_ICON_SIZE));
    icons::power(drawList, ImVec2(iconTopLeft.x + contentWidth * 0.5f, iconTopLeft.y + POWER_ICON_SIZE * 0.5f),
                 POWER_ICON_SIZE, theme::ACCENT);

    ImGui::Spacing();
    ImGui::PushFont(fonts::semibold(), 18.0f);
    char heading[64];
    std::snprintf(heading, sizeof(heading), "Shut down %s?", theme::OS_NAME);
    const float headingWidth = ImGui::CalcTextSize(heading).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (contentWidth - headingWidth) * 0.5f);
    ImGui::TextUnformatted(heading);
    ImGui::PopFont();

    const char* const note = "Any open apps will close.";
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (contentWidth - ImGui::CalcTextSize(note).x) * 0.5f);
    ImGui::TextDisabled("%s", note);
    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Cancel", POWER_BUTTON_SIZE) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (theme::accentButton("Shut Down", POWER_BUTTON_SIZE)) {
        shutdownConfirmed_ = true;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}
