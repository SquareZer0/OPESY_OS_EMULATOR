#include "screens/LoadingScreen.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>

#include "imgui.h"
#include "screens/ScreenUtil.h"
#include "ui/Fonts.h"
#include "ui/Theme.h"

namespace {

constexpr double DURATION_SECONDS = 3.0;
constexpr double HOLD_AT_FULL_SECONDS = 0.3;  // show 100% briefly before moving on

const ImVec4 BACKGROUND(17 / 255.0f, 31 / 255.0f, 42 / 255.0f, 1.0f);  // dark navy
constexpr ImU32 TEXT_COLOR = IM_COL32(236, 240, 244, 255);
constexpr ImU32 BAR_COLOR = IM_COL32(220, 225, 230, 255);

constexpr float TITLE_SIZE = 64.0f;
constexpr float SUBTITLE_SIZE = 22.0f;
constexpr float TEXT_SIZE = 18.0f;

constexpr float BAR_WIDTH = 230.0f;
constexpr float BAR_HEIGHT = 30.0f;
constexpr float BAR_BORDER = 3.0f;
constexpr int SEGMENTS = 16;
constexpr float SEGMENT_GAP = 3.0f;

struct Status {
    float until;  // shown while progress is below this
    const char* text;
};

const Status STATUSES[] = {
    {0.30f, "Initializing system services..."},
    {0.60f, "Loading drivers..."},
    {0.85f, "Starting user interface..."},
    {1.01f, "Preparing desktop..."},
};

// Padded to the same width so the centered text doesn't shift as dots appear.
const char* const LOADING_FRAMES[] = {"Loading   ", "Loading.  ", "Loading.. ", "Loading..."};
constexpr double LOADING_FRAMES_PER_SECOND = 3.0;

void textCenteredAt(ImDrawList* drawList, ImFont* font, float size, float centerX, float y, const char* text) {
    const float width = font->CalcTextSizeA(size, FLT_MAX, 0.0f, text).x;
    drawList->AddText(font, size, ImVec2(centerX - width * 0.5f, y), TEXT_COLOR, text);
}

}  // namespace

void LoadingScreen::render(double elapsed) {
    beginFullscreenWindow("##loading", BACKGROUND);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Positions are fractions of the window height, so the layout follows resizes.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float centerX = viewport->GetCenter().x;
    const float top = viewport->Pos.y;
    const float height = viewport->Size.y;

    textCenteredAt(drawList, fonts::semibold(), TITLE_SIZE, centerX, top + height * 0.16f, theme::OS_NAME);
    textCenteredAt(drawList, fonts::mono(), SUBTITLE_SIZE, centerX, top + height * 0.33f, "Desktop Operating System");
    textCenteredAt(drawList, fonts::mono(), SUBTITLE_SIZE, centerX, top + height * 0.38f, "v1.0");

    // Eases in and out, and reaches 100% a moment before the screen ends.
    const float t = static_cast<float>(std::min(1.0, elapsed / (DURATION_SECONDS - HOLD_AT_FULL_SECONDS)));
    const float progress = t * t * (3.0f - 2.0f * t);

    const char* status = STATUSES[0].text;
    for (const Status& s : STATUSES) {
        if (progress < s.until) {
            status = s.text;
            break;
        }
    }
    textCenteredAt(drawList, fonts::mono(), TEXT_SIZE, centerX, top + height * 0.62f, status);

    // Segmented bar with the percentage to its right; the pair is centered together.
    char percent[8];
    std::snprintf(percent, sizeof(percent), "%d%%", static_cast<int>(progress * 100.0f));
    const float percentGap = 14.0f;
    const float percentWidth = fonts::mono()->CalcTextSizeA(TEXT_SIZE, FLT_MAX, 0.0f, "100%").x;
    const ImVec2 barMin(centerX - (BAR_WIDTH + percentGap + percentWidth) * 0.5f, top + height * 0.72f);
    const ImVec2 barMax(barMin.x + BAR_WIDTH, barMin.y + BAR_HEIGHT);
    drawList->AddRect(barMin, barMax, BAR_COLOR, 2.0f, 0, BAR_BORDER);

    const float inset = BAR_BORDER + 3.0f;
    const float segmentWidth = (BAR_WIDTH - inset * 2.0f - SEGMENT_GAP * (SEGMENTS - 1)) / SEGMENTS;
    const int filled = static_cast<int>(progress * SEGMENTS + 0.0001f);
    for (int i = 0; i < filled; ++i) {
        const float x = barMin.x + inset + i * (segmentWidth + SEGMENT_GAP);
        drawList->AddRectFilled(ImVec2(x, barMin.y + inset), ImVec2(x + segmentWidth, barMax.y - inset), BAR_COLOR,
                                1.0f);
    }
    drawList->AddText(fonts::mono(), TEXT_SIZE,
                      ImVec2(barMax.x + percentGap, (barMin.y + barMax.y - TEXT_SIZE) * 0.5f), TEXT_COLOR, percent);

    const char* loading = LOADING_FRAMES[static_cast<int>(elapsed * LOADING_FRAMES_PER_SECOND) % 4];
    textCenteredAt(drawList, fonts::mono(), TEXT_SIZE, centerX, top + height * 0.82f, loading);

    ImGui::End();
}

bool LoadingScreen::isDone(double elapsed) const {
    return elapsed >= DURATION_SECONDS;
}
