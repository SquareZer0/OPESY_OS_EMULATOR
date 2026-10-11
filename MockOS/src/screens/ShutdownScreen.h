#pragma once

// "Shutting down..." screen shown after PWR is confirmed, right before the app exits.
class ShutdownScreen {
public:
    // `elapsed` is the number of seconds since this screen appeared.
    void render(double elapsed);
    bool isDone(double elapsed) const;
};
