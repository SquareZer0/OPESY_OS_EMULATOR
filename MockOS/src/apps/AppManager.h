#pragma once

#include <memory>
#include <vector>

#include "apps/App.h"

// The one list of apps. The taskbar, Task Manager, and each app's own close
// button all read and change the same open/closed state through it.
class AppManager {
public:
    AppManager();
    // Task Manager keeps a reference to this object, so it must never be copied or moved.
    AppManager(const AppManager&) = delete;
    AppManager& operator=(const AppManager&) = delete;

    const std::vector<std::unique_ptr<App>>& list() const { return apps_; }

    // Draws every open app window.
    void render();

    // Taskbar button behavior, like Windows: open a closed app, bring a
    // background app to the front, close the app that is already in front.
    void onTaskbarClick(App& app);

private:
    std::vector<std::unique_ptr<App>> apps_;
    App* frontApp_ = nullptr;  // the app window that most recently had focus
};
