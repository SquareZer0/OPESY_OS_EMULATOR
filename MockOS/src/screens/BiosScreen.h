#pragma once

// Text-console BIOS screen shown for a few seconds at startup.
class BiosScreen {
public:
    // `elapsed` is the number of seconds since this screen appeared.
    void render(double elapsed);
    bool isDone(double elapsed) const;
};
