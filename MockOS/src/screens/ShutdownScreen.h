#pragma once

// "Shutting down..." screen shown after PWR is confirmed, right before the app exits.
class ShutdownScreen {
public:
    void render();
    // `elapsed` is the number of seconds since this screen appeared.
    bool isDone(double elapsed) const;
};
