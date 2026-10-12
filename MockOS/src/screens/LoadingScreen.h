#pragma once

// Dark boot-loader screen with a segmented progress bar, shown between the
// BIOS and the desktop.
class LoadingScreen {
public:
    // `elapsed` is the number of seconds since this screen appeared.
    void render(double elapsed);
    bool isDone(double elapsed) const;
};
