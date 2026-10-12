#pragma once

#include "imgui.h"
#include "ui/Icons.h"

// The pe$OS look: flat and pastel, with translucent "frosted glass" panels.
// Shared colors, sizes and drawing helpers, so every screen and app matches.
namespace theme {

const char* const OS_NAME = "pe$OS";

constexpr float TOP_BAR_HEIGHT = 32.0f;

constexpr ImU32 TEXT = IM_COL32(59, 63, 81, 255);          // dark slate
constexpr ImU32 TEXT_MUTED = IM_COL32(112, 116, 138, 255);
constexpr ImU32 ACCENT = IM_COL32(150, 130, 230, 255);     // lavender
constexpr ImU32 GLASS_FILL = IM_COL32(255, 255, 255, 140);
constexpr ImU32 GLASS_BORDER = IM_COL32(255, 255, 255, 200);
constexpr ImU32 HOVER_FILL = IM_COL32(255, 255, 255, 130);

// Applies the pastel glass style to every ImGui widget. Call once at startup,
// after the fonts are loaded.
void apply();

// Soft drop shadow just outside a rectangle. Not clipped to the current window.
void drawShadow(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float rounding);

// Frosted-glass panel: shadow, translucent white fill, and a bright edge.
void drawGlassPanel(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float rounding);

// A button in the accent color, for the main action in a dialog.
bool accentButton(const char* label, const ImVec2& size = ImVec2(0.0f, 0.0f));

// A glass card with a child window inside it. Pair with ImGui::EndChild().
void beginGlassCard(const char* id, const ImVec2& size);

// One icon-and-label row of a sidebar card. The active row is highlighted;
// disabled rows are greyed out and can't be clicked. Returns true on click.
bool sidebarItem(const char* label, icons::LineIconFn icon, bool active, bool enabled = true);

}  // namespace theme
