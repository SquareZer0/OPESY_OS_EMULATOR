#pragma once

#include <string>

#include "imgui.h"
#include "ui/Icons.h"

// Base class for every widget that opens from the taskbar. Subclasses only
// draw their contents; App handles the window, its close button, and focus.
class App {
public:
    App(std::string name, std::string processName, icons::AppIconFn icon, ImVec2 defaultPos, ImVec2 defaultSize);
    virtual ~App() = default;

    const std::string& name() const { return name_; }                // window title and taskbar tooltip
    const std::string& processName() const { return processName_; }  // shown in Task Manager
    bool isOpen() const { return open_; }

    // Draws this app's taskbar icon in the square at `min`.
    void drawIcon(ImDrawList* drawList, const ImVec2& min, float size) const { icon_(drawList, min, size); }

    // Opens the window if it's closed, and brings it to the front either way.
    void open();
    void close();

    // Draws the window if it's open. Returns true if the window has focus.
    bool render();

protected:
    // Everything inside the window. Called between ImGui::Begin() and ImGui::End().
    virtual void renderContents() = 0;

    // Extra ImGui window flags, e.g. ImGuiWindowFlags_MenuBar.
    virtual ImGuiWindowFlags windowFlags() const { return 0; }

private:
    std::string name_;
    std::string processName_;
    icons::AppIconFn icon_;
    ImVec2 defaultPos_;
    ImVec2 defaultSize_;
    bool open_ = false;
    bool focusRequested_ = false;
};
