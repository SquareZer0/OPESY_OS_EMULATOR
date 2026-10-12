#pragma once

class AppManager;

// Frosted bar along the top of the desktop. Left: the OS name and the app in
// front. Right: Wi-Fi and sound icons, the clock, and the power (PWR) button.
class TopBar {
public:
    // Returns true on the frame the power button is clicked.
    bool render(const AppManager& apps);

private:
    void renderClock(float rightEdge, float& x);

    bool muted_ = false;  // mock: clicking the speaker icon toggles it
};
