#pragma once

// Logo and loading animation shown between the BIOS and the desktop.
class SplashScreen {
public:
    // `elapsed` is the number of seconds since this screen appeared.
    void render(double elapsed);
    bool isDone(double elapsed) const;
};
