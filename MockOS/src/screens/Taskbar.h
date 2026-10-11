#pragma once

class AppManager;

// Bar along the bottom of the desktop: PWR button, one button per app, and the clock.
class Taskbar {
public:
    static constexpr float HEIGHT = 44.0f;

    // Returns true on the frame the PWR button is clicked.
    bool render(AppManager& apps);

private:
    void renderClock();
};
