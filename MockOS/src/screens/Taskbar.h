#pragma once

#include <vector>

class AppManager;

// The taskbar, drawn as a floating dock: one icon button per app, with a dot
// under each app that is running.
class Taskbar {
public:
    void render(AppManager& apps);

private:
    std::vector<float> hover_;  // per icon, 0 = resting, 1 = fully raised; animated
};
