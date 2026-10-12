#include "screens/Taskbar.h"

#include <algorithm>

#include "apps/AppManager.h"
#include "imgui.h"
#include "screens/ScreenUtil.h"
#include "ui/Theme.h"

namespace {

constexpr float ICON_SIZE = 48.0f;
constexpr float ICON_GAP = 14.0f;
constexpr float SIDE_PADDING = 16.0f;
constexpr float TOP_PADDING = 10.0f;
constexpr float DOCK_HEIGHT = 74.0f;  // icon, running dot, and padding
constexpr float DOT_OFFSET = 7.0f;   // from the icon's bottom edge to the running dot
constexpr float BOTTOM_MARGIN = 12.0f;
constexpr float ROUNDING = 22.0f;
constexpr float HOVER_LIFT = 6.0f;
constexpr float HOVER_GROW = 4.0f;
constexpr float HOVER_SPEED = 14.0f;  // higher = snappier raise animation

}  // namespace

void Taskbar::render(AppManager& apps) {
    const auto& list = apps.list();
    const int count = static_cast<int>(list.size());
    hover_.resize(list.size(), 0.0f);

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float dockWidth = SIDE_PADDING * 2.0f + count * ICON_SIZE + (count - 1) * ICON_GAP;
    // The window is taller than the dock, leaving room for raised icons.
    const ImVec2 windowSize(dockWidth, DOCK_HEIGHT + HOVER_LIFT + HOVER_GROW);
    const ImVec2 windowPos(viewport->Pos.x + (viewport->Size.x - dockWidth) * 0.5f,
                           viewport->Pos.y + viewport->Size.y - BOTTOM_MARGIN - windowSize.y);
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##taskbar", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar(2);
    keepCurrentWindowOnTop();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 dockMin(windowPos.x, windowPos.y + windowSize.y - DOCK_HEIGHT);
    const ImVec2 dockMax(dockMin.x + dockWidth, dockMin.y + DOCK_HEIGHT);
    theme::drawGlassPanel(drawList, dockMin, dockMax, ROUNDING);

    const float step = std::min(1.0f, ImGui::GetIO().DeltaTime * HOVER_SPEED);
    for (int i = 0; i < count; ++i) {
        App& app = *list[i];
        const float iconX = dockMin.x + SIDE_PADDING + i * (ICON_SIZE + ICON_GAP);
        const float iconY = dockMin.y + TOP_PADDING;

        ImGui::SetCursorScreenPos(ImVec2(iconX, iconY));
        ImGui::PushID(i);
        if (ImGui::InvisibleButton("##app", ImVec2(ICON_SIZE, ICON_SIZE))) {
            apps.onTaskbarClick(app);
        }
        const bool hovered = ImGui::IsItemHovered();
        ImGui::SetItemTooltip("%s", app.name().c_str());
        ImGui::PopID();

        // Hovered icons rise and grow a little.
        hover_[i] += ((hovered ? 1.0f : 0.0f) - hover_[i]) * step;
        const float grow = HOVER_GROW * hover_[i];
        const float size = ICON_SIZE + grow;
        app.drawIcon(drawList, ImVec2(iconX - grow * 0.5f, iconY - HOVER_LIFT * hover_[i] - grow), size);

        if (app.isOpen()) {
            drawList->AddCircleFilled(ImVec2(iconX + ICON_SIZE * 0.5f, iconY + ICON_SIZE + DOT_OFFSET), 2.5f,
                                      theme::TEXT);
        }
    }

    ImGui::End();
}
