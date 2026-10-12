#pragma once

#include "imgui.h"

// Icons drawn with ImGui shapes instead of image files, so they stay sharp at
// any size and need no assets.
namespace icons {

// Line icons for the top bar, centered on `center` inside a `size` x `size` box.
void wifi(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void speaker(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color, bool muted);
void power(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);

// Line icons for the Task Manager sidebar, laid out like the top bar icons.
using LineIconFn = void (*)(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void processes(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void performance(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void appHistory(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void startupApps(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void details(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void services(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);

// Line icons for the browser toolbar and search box.
void arrowLeft(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void arrowRight(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void reload(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void home(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void lock(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void star(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void profile(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void moreVertical(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void search(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);
void mic(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color);

// The red, yellow, green and blue Chrome logo, `size` across.
void chromeLogo(ImDrawList* drawList, const ImVec2& center, float size);

// App icons: a rounded pastel tile with a white symbol, filling the square
// whose top-left corner is `min`. Each app passes one of these to App.
using AppIconFn = void (*)(ImDrawList* drawList, const ImVec2& min, float size);
void chrome(ImDrawList* drawList, const ImVec2& min, float size);  // the colored logo on a white tile
void calculator(ImDrawList* drawList, const ImVec2& min, float size);
void notepad(ImDrawList* drawList, const ImVec2& min, float size);
void taskManager(ImDrawList* drawList, const ImVec2& min, float size);

}  // namespace icons
