#include "screens/Desktop.h"

#include "imgui.h"

namespace {

const char* const POWER_POPUP_ID = "Shut Down";
const ImVec2 POWER_BUTTON_SIZE(120.0f, 0.0f);

}  // namespace

void Desktop::render() {
    renderWallpaper();
    apps_.render();
    if (taskbar_.render(apps_)) {
        requestPowerDialog();
    }
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

void Desktop::renderWallpaper() {
    // Drawn on the background draw list so it sits behind every window, and
    // sized from the viewport every frame so it follows window resizes.
    // PLACEHOLDER (Phase 2): replace with your own wallpaper design.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 min = viewport->Pos;
    const ImVec2 max(min.x + viewport->Size.x, min.y + viewport->Size.y);
    const ImU32 top = IM_COL32(32, 58, 110, 255);
    const ImU32 bottom = IM_COL32(12, 20, 40, 255);
    ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(min, max, top, top, bottom, bottom);
}

void Desktop::renderPowerDialog() {
    // OpenPopup() has to be called from the same place as BeginPopupModal(),
    // so the taskbar and the X button only set a flag and the popup opens here.
    if (powerDialogRequested_) {
        ImGui::OpenPopup(POWER_POPUP_ID);
        powerDialogRequested_ = false;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(POWER_POPUP_ID, nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::TextUnformatted("Are you sure you want to shut down MockOS?");
        ImGui::Spacing();
        if (ImGui::Button("Shut down", POWER_BUTTON_SIZE)) {
            shutdownConfirmed_ = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", POWER_BUTTON_SIZE) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
