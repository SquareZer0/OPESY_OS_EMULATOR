#include "apps/Browser.h"

#include <algorithm>
#include <cfloat>

#include "misc/cpp/imgui_stdlib.h"
#include "ui/Fonts.h"
#include "ui/Theme.h"
#include "ui/Wallpaper.h"

namespace {

constexpr float TOOLBAR_HEIGHT = 44.0f;
constexpr float BOTTOM_STRIP_HEIGHT = 22.0f;
constexpr float ICON_SIZE = 16.0f;
constexpr float BUTTON_SIZE = 28.0f;
constexpr float BUTTON_STEP = 32.0f;
constexpr float LOGO_SIZE = 20.0f;
constexpr float ADDRESS_HEIGHT = 28.0f;
constexpr float ADDRESS_FONT_SIZE = 14.0f;
constexpr float GOOGLE_FONT_SIZE = 60.0f;
constexpr float SEARCH_HEIGHT = 42.0f;
constexpr float SEARCH_MAX_WIDTH = 560.0f;
constexpr float SEARCH_FONT_SIZE = 17.0f;

constexpr ImU32 TOOLBAR_FILL = IM_COL32(236, 234, 242, 235);
constexpr ImU32 STRIP_FILL = IM_COL32(255, 255, 255, 150);
constexpr ImU32 PAGE_FROST = IM_COL32(255, 255, 255, 70);
constexpr ImU32 FIELD_FILL = IM_COL32(255, 255, 255, 235);
constexpr ImU32 ICON_COLOR = IM_COL32(50, 52, 60, 255);
constexpr ImU32 SEARCH_ICON_COLOR = IM_COL32(120, 124, 140, 255);
constexpr ImU32 HOVER_FILL = IM_COL32(0, 0, 0, 22);
constexpr ImU32 GOOGLE_SHADOW = IM_COL32(90, 80, 130, 24);

// A round toolbar button with a hover highlight. The mock browser can't
// navigate, so clicking does nothing.
void toolbarButton(const char* id, const ImVec2& center, icons::LineIconFn icon) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImGui::SetCursorScreenPos(ImVec2(center.x - BUTTON_SIZE * 0.5f, center.y - BUTTON_SIZE * 0.5f));
    ImGui::InvisibleButton(id, ImVec2(BUTTON_SIZE, BUTTON_SIZE));
    if (ImGui::IsItemHovered()) {
        drawList->AddCircleFilled(center, BUTTON_SIZE * 0.5f, HOVER_FILL);
    }
    icon(drawList, center, ICON_SIZE, ICON_COLOR);
}

}  // namespace

Browser::Browser() : App("Chrome", "chrome.exe", icons::chrome, ImVec2(230.0f, 60.0f), ImVec2(820.0f, 520.0f)) {}

void Browser::renderContents() {
    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();
    const float border = ImGui::GetStyle().WindowBorderSize;
    const float left = windowPos.x + border;
    const float right = windowPos.x + windowSize.x - border;
    const float bottom = windowPos.y + windowSize.y - border;
    // The content area starts under the title bar. The window padding is
    // skipped so the toolbar, page and bottom strip reach the window's edges.
    const float top = ImGui::GetCursorScreenPos().y - ImGui::GetStyle().WindowPadding.y;
    const float toolbarBottom = top + TOOLBAR_HEIGHT;
    const float stripTop = bottom - BOTTOM_STRIP_HEIGHT;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->PushClipRect(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), false);
    wallpaper::draw(drawList, ImVec2(left, toolbarBottom), ImVec2(right, stripTop));
    drawList->AddRectFilled(ImVec2(left, toolbarBottom), ImVec2(right, stripTop), PAGE_FROST);
    drawList->AddRectFilled(ImVec2(left, top), ImVec2(right, toolbarBottom), TOOLBAR_FILL);
    drawList->AddRectFilled(ImVec2(left, stripTop), ImVec2(right, bottom), STRIP_FILL,
                            ImGui::GetStyle().WindowRounding - border, ImDrawFlags_RoundCornersBottom);
    drawList->PopClipRect();

    renderToolbar(left, right, top);
    renderPage(ImVec2(left, toolbarBottom), ImVec2(right, stripTop));
}

// Logo, back / forward / reload / home, the address bar, then profile and menu.
void Browser::renderToolbar(float left, float right, float top) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float padding = ImGui::GetStyle().WindowPadding.x;
    const float centerY = top + TOOLBAR_HEIGHT * 0.5f;

    float x = left + padding + LOGO_SIZE * 0.5f;
    icons::chromeLogo(drawList, ImVec2(x, centerY), LOGO_SIZE);
    x += BUTTON_STEP;
    toolbarButton("##back", ImVec2(x, centerY), icons::arrowLeft);
    x += BUTTON_STEP;
    toolbarButton("##forward", ImVec2(x, centerY), icons::arrowRight);
    x += BUTTON_STEP;
    toolbarButton("##reload", ImVec2(x, centerY), icons::reload);
    x += BUTTON_STEP;
    toolbarButton("##home", ImVec2(x, centerY), icons::home);

    const float moreX = right - padding - BUTTON_SIZE * 0.5f;
    const float profileX = moreX - BUTTON_STEP;
    toolbarButton("##profile", ImVec2(profileX, centerY), icons::profile);
    toolbarButton("##more", ImVec2(moreX, centerY), icons::moreVertical);

    // Address bar: a lock, the URL and a star. It's read-only.
    const ImVec2 barMin(x + BUTTON_SIZE * 0.5f + 8.0f, centerY - ADDRESS_HEIGHT * 0.5f);
    const ImVec2 barMax(profileX - BUTTON_SIZE * 0.5f - 8.0f, centerY + ADDRESS_HEIGHT * 0.5f);
    if (barMax.x - barMin.x < 80.0f) {
        return;  // window too narrow to fit it
    }
    drawList->AddRectFilled(barMin, barMax, FIELD_FILL, ADDRESS_HEIGHT * 0.5f);
    icons::lock(drawList, ImVec2(barMin.x + 18.0f, centerY), 12.0f, ICON_COLOR);
    drawList->AddText(ImGui::GetFont(), ADDRESS_FONT_SIZE, ImVec2(barMin.x + 34.0f, centerY - ADDRESS_FONT_SIZE * 0.55f),
                      theme::TEXT, "google.com");
    icons::star(drawList, ImVec2(barMax.x - 18.0f, centerY), 15.0f, ICON_COLOR);
}

// The Google logo and the "Ask Google" search box, centered on the page.
void Browser::renderPage(const ImVec2& min, const ImVec2& max) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float centerX = (min.x + max.x) * 0.5f;
    const float height = max.y - min.y;

    ImFont* font = fonts::semibold();
    const char* word = "Google";
    const float wordWidth = font->CalcTextSizeA(GOOGLE_FONT_SIZE, FLT_MAX, 0.0f, word).x;
    const ImVec2 wordPos(centerX - wordWidth * 0.5f, min.y + height * 0.22f);
    for (int i = 1; i <= 3; ++i) {
        drawList->AddText(font, GOOGLE_FONT_SIZE, ImVec2(wordPos.x, wordPos.y + i * 1.5f), GOOGLE_SHADOW, word);
    }
    drawList->AddText(font, GOOGLE_FONT_SIZE, wordPos, IM_COL32(255, 255, 255, 255), word);

    const float width = std::min(SEARCH_MAX_WIDTH, (max.x - min.x) - 80.0f);
    const ImVec2 boxMin(centerX - width * 0.5f, min.y + height * 0.48f);
    const ImVec2 boxMax(boxMin.x + width, boxMin.y + SEARCH_HEIGHT);
    const float boxCenterY = (boxMin.y + boxMax.y) * 0.5f;
    theme::drawShadow(drawList, boxMin, boxMax, SEARCH_HEIGHT * 0.5f);
    drawList->AddRectFilled(boxMin, boxMax, FIELD_FILL, SEARCH_HEIGHT * 0.5f);
    icons::search(drawList, ImVec2(boxMin.x + 26.0f, boxCenterY), 20.0f, SEARCH_ICON_COLOR);
    icons::mic(drawList, ImVec2(boxMax.x - 26.0f, boxCenterY), 20.0f, SEARCH_ICON_COLOR);

    // A frameless text box sitting inside the drawn pill.
    ImGui::PushFont(nullptr, SEARCH_FONT_SIZE);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
    ImGui::SetCursorScreenPos(ImVec2(boxMin.x + 46.0f, boxCenterY - ImGui::GetFrameHeight() * 0.5f));
    ImGui::SetNextItemWidth(width - 92.0f);
    ImGui::InputTextWithHint("##search", "Ask Google", &query_);
    ImGui::PopStyleColor(3);
    ImGui::PopFont();
}
