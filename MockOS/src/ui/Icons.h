#pragma once

#include "imgui.h"

// Icons drawn with ImGui shapes instead of image files, so they stay sharp at
// any size and need no assets.
namespace icons {

// Line icons for the top bar, centered on `center` inside a `size` x `size` box.
void wifi(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void speaker(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color, bool muted);
void power(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);

// App icons: a rounded pastel tile with a white symbol, filling the square
// whose top-left corner is `min`. Each app passes one of these to App.
using AppIconFn = void (*)(ImDrawList* drawList, const ImVec2& min, float size);
void calculator(ImDrawList* drawList, const ImVec2& min, float size);
void notepad(ImDrawList* drawList, const ImVec2& min, float size);
void taskManager(ImDrawList* drawList, const ImVec2& min, float size);

}  // namespace icons
