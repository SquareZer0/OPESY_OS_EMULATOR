#include "screens/TopBar.h"

#include <ctime>

#include "apps/AppManager.h"
#include "imgui.h"
#include "screens/ScreenUtil.h"
#include "ui/Fonts.h"
#include "ui/Icons.h"
#include "ui/Theme.h"

namespace {

constexpr float FONT_SIZE = 14.0f;
constexpr float SIDE_PADDING = 14.0f;
constexpr float ICON_BOX = 24.0f;  // clickable square around each icon
constexpr float ICON_SIZE = 16.0f;
constexpr float ITEM_GAP = 10.0f;

// A square button with a hover highlight. Returns true when clicked; the
// caller draws the icon at `center`.
bool iconButton(const char* id, float x, ImVec2& center) {
    ImGui::SetCursorPos(ImVec2(x, (theme::TOP_BAR_HEIGHT - ICON_BOX) * 0.5f));
    const bool clicked = ImGui::InvisibleButton(id, ImVec2(ICON_BOX, ICON_BOX));
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    if (ImGui::IsItemHovered()) {
        ImGui::GetWindowDrawList()->AddRectFilled(min, max, theme::HOVER_FILL, 6.0f);
    }
    center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
    return clicked;
}

void textAt(float x, const char* text) {
    ImGui::SetCursorPos(ImVec2(x, (theme::TOP_BAR_HEIGHT - ImGui::GetTextLineHeight()) * 0.5f));
    ImGui::TextUnformatted(text);
}

}  // namespace

bool TopBar::render(const AppManager& apps) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float width = viewport->Size.x;
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(width, theme::TOP_BAR_HEIGHT));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("##topbar", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar(3);
    keepCurrentWindowOnTop();

    // Glass bar with a bright bottom edge and a soft shadow below it.
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 min = viewport->Pos;
    const ImVec2 max(min.x + width, min.y + theme::TOP_BAR_HEIGHT);
    theme::drawShadow(drawList, min, max, 0.0f);
    drawList->AddRectFilled(min, max, theme::GLASS_FILL);
    drawList->AddLine(ImVec2(min.x, max.y - 0.5f), ImVec2(max.x, max.y - 0.5f), theme::GLASS_BORDER);

    // Left: OS name, then the app in front (like the macOS menu bar).
    ImGui::PushFont(fonts::semibold(), FONT_SIZE);
    textAt(SIDE_PADDING, theme::OS_NAME);
    const float nameEnd = SIDE_PADDING + ImGui::CalcTextSize(theme::OS_NAME).x;
    ImGui::PopFont();
    ImGui::PushFont(fonts::regular(), FONT_SIZE);
    if (const App* front = apps.frontApp()) {
        textAt(nameEnd + 18.0f, front->name().c_str());
    }

    // Right, laid out from the right edge inward: power, clock, sound, Wi-Fi.
    float x = width - SIDE_PADDING - ICON_BOX;
    ImVec2 center;
    const bool powerClicked = iconButton("##power", x, center);
    icons::power(drawList, center, ICON_SIZE, theme::TEXT);
    ImGui::SetItemTooltip("PWR: shut down");

    renderClock(x - ITEM_GAP, x);

    x -= ITEM_GAP + ICON_BOX;
    if (iconButton("##sound", x, center)) {
        muted_ = !muted_;
    }
    icons::speaker(drawList, center, ICON_SIZE, theme::TEXT, muted_);
    ImGui::SetItemTooltip(muted_ ? "Sound: muted" : "Sound: on");

    x -= ICON_BOX;
    iconButton("##wifi", x, center);
    icons::wifi(drawList, center, ICON_SIZE, theme::TEXT);
    ImGui::SetItemTooltip("Wi-Fi: connected");

    ImGui::PopFont();
    ImGui::End();
    return powerClicked;
}

void TopBar::renderClock(float rightEdge, float& x) {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    char time[16];
    std::strftime(time, sizeof(time), "%I:%M %p", &local);
    const char* shown = (time[0] == '0') ? time + 1 : time;  // "9:05 AM", not "09:05 AM"

    x = rightEdge - ImGui::CalcTextSize(shown).x;
    textAt(x, shown);
    if (ImGui::IsItemHovered()) {
        char date[64];
        std::strftime(date, sizeof(date), "%A, %B %d, %Y", &local);
        ImGui::SetTooltip("%s", date);
    }
}
