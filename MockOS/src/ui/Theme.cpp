#include "ui/Theme.h"

#include "ui/Fonts.h"

namespace theme {

namespace {

ImVec4 rgba(int r, int g, int b, int a) {
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

constexpr int SHADOW_LAYERS = 10;
constexpr int SHADOW_MAX_ALPHA = 24;

constexpr float CARD_ROUNDING = 12.0f;
constexpr float CARD_PADDING = 10.0f;
constexpr float SIDEBAR_ROW_HEIGHT = 32.0f;
constexpr float SIDEBAR_ICON_SIZE = 16.0f;
constexpr ImU32 ACTIVE_ROW_FILL = IM_COL32(150, 130, 230, 45);
constexpr ImU32 HOVERED_ROW_FILL = IM_COL32(255, 255, 255, 140);

}  // namespace

void apply() {
    ImGuiStyle& style = ImGui::GetStyle();
    // Start from the light theme so any color not set below still fits.
    ImGui::StyleColorsLight(&style);

    style.WindowRounding = 14.0f;
    style.ChildRounding = 10.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 12.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;
    style.WindowBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(14.0f, 12.0f);
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 8.0f);
    style.ScrollbarSize = 12.0f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = rgba(59, 63, 81, 255);
    colors[ImGuiCol_TextDisabled] = rgba(140, 144, 163, 255);
    // Mostly opaque, so text in a window behind doesn't show through too much.
    colors[ImGuiCol_WindowBg] = rgba(255, 255, 255, 205);
    colors[ImGuiCol_ChildBg] = rgba(255, 255, 255, 60);
    colors[ImGuiCol_PopupBg] = rgba(252, 250, 255, 235);
    colors[ImGuiCol_Border] = rgba(255, 255, 255, 220);
    colors[ImGuiCol_BorderShadow] = rgba(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg] = rgba(255, 255, 255, 150);
    colors[ImGuiCol_FrameBgHovered] = rgba(255, 255, 255, 210);
    colors[ImGuiCol_FrameBgActive] = rgba(238, 232, 255, 235);
    colors[ImGuiCol_TitleBg] = rgba(246, 243, 252, 225);
    colors[ImGuiCol_TitleBgActive] = rgba(255, 255, 255, 240);
    colors[ImGuiCol_TitleBgCollapsed] = rgba(246, 243, 252, 225);
    colors[ImGuiCol_MenuBarBg] = rgba(255, 255, 255, 110);
    colors[ImGuiCol_ScrollbarBg] = rgba(0, 0, 0, 0);
    colors[ImGuiCol_ScrollbarGrab] = rgba(160, 150, 200, 110);
    colors[ImGuiCol_ScrollbarGrabHovered] = rgba(160, 150, 200, 170);
    colors[ImGuiCol_ScrollbarGrabActive] = rgba(150, 130, 230, 220);
    colors[ImGuiCol_CheckMark] = rgba(150, 130, 230, 255);
    colors[ImGuiCol_SliderGrab] = rgba(150, 130, 230, 200);
    colors[ImGuiCol_SliderGrabActive] = rgba(150, 130, 230, 255);
    colors[ImGuiCol_Button] = rgba(255, 255, 255, 170);
    colors[ImGuiCol_ButtonHovered] = rgba(255, 255, 255, 235);
    colors[ImGuiCol_ButtonActive] = rgba(220, 210, 252, 255);
    colors[ImGuiCol_Header] = rgba(196, 184, 245, 120);
    colors[ImGuiCol_HeaderHovered] = rgba(196, 184, 245, 170);
    colors[ImGuiCol_HeaderActive] = rgba(180, 165, 240, 220);
    colors[ImGuiCol_Separator] = rgba(120, 110, 160, 60);
    colors[ImGuiCol_SeparatorHovered] = rgba(150, 130, 230, 150);
    colors[ImGuiCol_SeparatorActive] = rgba(150, 130, 230, 255);
    colors[ImGuiCol_ResizeGrip] = rgba(150, 130, 230, 40);
    colors[ImGuiCol_ResizeGripHovered] = rgba(150, 130, 230, 120);
    colors[ImGuiCol_ResizeGripActive] = rgba(150, 130, 230, 200);
    colors[ImGuiCol_PlotLines] = rgba(150, 130, 230, 255);
    colors[ImGuiCol_PlotHistogram] = rgba(150, 130, 230, 255);
    colors[ImGuiCol_TableHeaderBg] = rgba(255, 255, 255, 170);
    colors[ImGuiCol_TableBorderStrong] = rgba(120, 110, 160, 70);
    colors[ImGuiCol_TableBorderLight] = rgba(120, 110, 160, 40);
    colors[ImGuiCol_TableRowBg] = rgba(0, 0, 0, 0);
    colors[ImGuiCol_TableRowBgAlt] = rgba(255, 255, 255, 80);
    colors[ImGuiCol_TextSelectedBg] = rgba(150, 130, 230, 90);
    colors[ImGuiCol_ModalWindowDimBg] = rgba(90, 80, 130, 60);
}

void drawShadow(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float rounding) {
    // Rings that fade out as they move away from the edge. They extend
    // further below than above, as if lit from the top.
    drawList->PushClipRectFullScreen();
    for (int i = 1; i <= SHADOW_LAYERS; ++i) {
        const float fade = 1.0f - static_cast<float>(i) / SHADOW_LAYERS;
        const int alpha = static_cast<int>(SHADOW_MAX_ALPHA * fade * fade);
        const float grow = static_cast<float>(i);
        drawList->AddRect(ImVec2(min.x - grow, min.y - grow * 0.6f), ImVec2(max.x + grow, max.y + grow * 1.4f),
                          IM_COL32(70, 60, 110, alpha), rounding + grow, 0, 1.5f);
    }
    drawList->PopClipRect();
}

void drawGlassPanel(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float rounding) {
    drawShadow(drawList, min, max, rounding);
    drawList->AddRectFilled(min, max, GLASS_FILL, rounding);
    drawList->AddRect(min, max, GLASS_BORDER, rounding, 0, 1.0f);
}

bool accentButton(const char* label, const ImVec2& size) {
    ImGui::PushStyleColor(ImGuiCol_Button, rgba(150, 130, 230, 235));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, rgba(166, 148, 238, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, rgba(132, 112, 215, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, rgba(255, 255, 255, 255));
    const bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleColor(4);
    return clicked;
}

void beginGlassCard(const char* id, const ImVec2& size) {
    const ImVec2 min = ImGui::GetCursorScreenPos();
    drawGlassPanel(ImGui::GetWindowDrawList(), min, ImVec2(min.x + size.x, min.y + size.y), CARD_ROUNDING);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(CARD_PADDING, CARD_PADDING));
    ImGui::BeginChild(id, size, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar();
}

bool sidebarItem(const char* label, icons::LineIconFn icon, bool active, bool enabled) {
    const float width = ImGui::GetContentRegionAvail().x;
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 max(min.x + width, min.y + SIDEBAR_ROW_HEIGHT);

    ImGui::PushID(label);
    ImGui::BeginDisabled(!enabled);
    const bool clicked = ImGui::InvisibleButton("##row", ImVec2(width, SIDEBAR_ROW_HEIGHT));
    const bool hovered = ImGui::IsItemHovered();
    ImGui::EndDisabled();
    ImGui::PopID();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (active || hovered) {
        drawList->AddRectFilled(min, max, active ? ACTIVE_ROW_FILL : HOVERED_ROW_FILL,
                                ImGui::GetStyle().FrameRounding);
    }
    const ImU32 textColor = enabled ? TEXT : ImGui::GetColorU32(ImGuiCol_TextDisabled);
    const float centerY = (min.y + max.y) * 0.5f;
    icon(drawList, ImVec2(min.x + 18.0f, centerY), SIDEBAR_ICON_SIZE, active ? ACCENT : textColor);
    const float fontSize = ImGui::GetFontSize();
    drawList->AddText(active ? fonts::semibold() : ImGui::GetFont(), fontSize,
                      ImVec2(min.x + 36.0f, centerY - fontSize * 0.5f), textColor, label);
    return clicked;
}

}  // namespace theme
