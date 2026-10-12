#include "apps/App.h"

#include <algorithm>
#include <utility>

#include "ui/Theme.h"

App::App(std::string name, std::string processName, icons::AppIconFn icon, ImVec2 defaultPos, ImVec2 defaultSize,
         ImVec2 minSize)
    : name_(std::move(name)),
      processName_(std::move(processName)),
      icon_(icon),
      defaultPos_(defaultPos),
      defaultSize_(defaultSize),
      minSize_(minSize),
      pos_(defaultPos) {}

void App::open() {
    open_ = true;
    focusRequested_ = true;
}

void App::close() {
    open_ = false;
}

bool App::render() {
    if (!open_) {
        return false;
    }
    if (focusRequested_) {
        ImGui::SetNextWindowFocus();
        focusRequested_ = false;
    }
    ImGui::SetNextWindowPos(defaultPos_, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(defaultSize_, ImGuiCond_FirstUseEver);
    // Resizing stops at the desktop's right and bottom edges, but never below the minimum.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 maxSize(std::max(minSize_.x, viewport->Pos.x + viewport->Size.x - pos_.x),
                         std::max(minSize_.y, viewport->Pos.y + viewport->Size.y - pos_.y));
    ImGui::SetNextWindowSizeConstraints(minSize_, maxSize);

    // Passing &open_ adds the window's X button, which sets open_ to false.
    const bool visible = ImGui::Begin(name_.c_str(), &open_, ImGuiWindowFlags_NoCollapse | windowFlags());
    const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    // Don't let the title bar slide under the top bar, where it couldn't be grabbed again.
    ImVec2 pos = ImGui::GetWindowPos();
    const float minY = ImGui::GetMainViewport()->Pos.y + theme::TOP_BAR_HEIGHT;
    if (pos.y < minY) {
        pos.y = minY;
        ImGui::SetWindowPos(pos);
    }
    pos_ = pos;
    const ImVec2 size = ImGui::GetWindowSize();
    theme::drawShadow(ImGui::GetWindowDrawList(), pos, ImVec2(pos.x + size.x, pos.y + size.y),
                      ImGui::GetStyle().WindowRounding);

    if (visible) {
        renderContents();
    }
    ImGui::End();
    return focused;
}
