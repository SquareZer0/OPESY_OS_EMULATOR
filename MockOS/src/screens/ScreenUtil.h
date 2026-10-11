#pragma once

#include "imgui.h"
#include "imgui_internal.h"  // BringWindowToDisplayFront, GetCurrentWindow

// Begins a borderless ImGui window that covers the whole app window and
// follows it when resized. Pass a transparent background to draw over the
// wallpaper. Always pair with ImGui::End().
inline void beginFullscreenWindow(const char* name, const ImVec4& background) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, background);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin(name, nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

// Draws one line of text horizontally centered in the current window.
inline void textCentered(const char* text) {
    const float width = ImGui::CalcTextSize(text).x;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - width) * 0.5f);
    ImGui::TextUnformatted(text);
}

// Keeps the current window (the top bar or the dock) drawn above app windows,
// which ImGui otherwise brings to the front when clicked. Skipped while a
// popup such as the shutdown dialog is open, since that must stay on top.
inline void keepCurrentWindowOnTop() {
    if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopup)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    }
}
