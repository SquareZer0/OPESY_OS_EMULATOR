#pragma once

#include <string>
#include <vector>

#include "system/SystemInfo.h"

// Text-console BIOS screen shown for a few seconds at startup. It shows the
// real CPU, memory, mainboard, graphics card and drives of the PC it runs on.
class BiosScreen {
public:
    // `elapsed` is the number of seconds since this screen appeared.
    void render(double elapsed);
    bool isDone(double elapsed) const;

private:
    enum class LineKind {
        Header,      // bright text
        Text,
        MemoryTest,  // counts up to the installed RAM, then says OK
        Dots,        // a row of dots that fills in
    };

    struct Line {
        double showAt;  // seconds after the screen appears
        LineKind kind;
        std::string text;
    };

    // Reads the hardware details and lays out the timeline. Runs on the first
    // frame, once the OpenGL context exists.
    void build();
    void renderLogo();

    bool built_ = false;
    SystemInfo info_;
    std::vector<Line> lines_;
    double doneAt_ = 0.0;
};
