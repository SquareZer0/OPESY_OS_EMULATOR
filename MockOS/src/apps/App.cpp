#include "apps/App.h"

#include <utility>

App::App(std::string name, std::string processName, ImVec2 defaultPos, ImVec2 defaultSize)
    : name_(std::move(name)),
      processName_(std::move(processName)),
      defaultPos_(defaultPos),
      defaultSize_(defaultSize) {}

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

    // Passing &open_ adds the window's X button, which sets open_ to false.
    const bool visible = ImGui::Begin(name_.c_str(), &open_);
    const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    if (visible) {
        renderContents();
    }
    ImGui::End();
    return focused;
}
