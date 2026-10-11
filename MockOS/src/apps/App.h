#pragma once

#include <string>

#include "imgui.h"

// Base class for every widget that opens from the taskbar. Subclasses only
// draw their contents; App handles the window, its close button, and focus.
class App {
public:
    App(std::string name, std::string processName, ImVec2 defaultPos, ImVec2 defaultSize);
    virtual ~App() = default;

    const std::string& name() const { return name_; }                // window title and taskbar label
    const std::string& processName() const { return processName_; }  // shown in Task Manager
    bool isOpen() const { return open_; }

    // Opens the window if it's closed, and brings it to the front either way.
    void open();
    void close();

    // Draws the window if it's open. Returns true if the window has focus.
    bool render();

protected:
    // Everything inside the window. Called between ImGui::Begin() and ImGui::End().
    virtual void renderContents() = 0;

private:
    std::string name_;
    std::string processName_;
    ImVec2 defaultPos_;
    ImVec2 defaultSize_;
    bool open_ = false;
    bool focusRequested_ = false;
};
