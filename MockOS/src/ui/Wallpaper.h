#pragma once

#include "imgui.h"

namespace wallpaper {

// Draws the pastel gradient wallpaper over the rectangle [min, max].
void draw(ImDrawList* drawList, const ImVec2& min, const ImVec2& max);

}  // namespace wallpaper
