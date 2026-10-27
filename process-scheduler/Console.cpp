#include "Console.h"

#include <cstdio>
#include <iostream>

#ifndef NOMINMAX
#define NOMINMAX  // stop windows.h from defining min/max macros that clash with std::min/std::max
#endif
#include <windows.h>

// Older Windows SDK headers don't define this.
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

namespace console {

const char* const GREEN = "\033[32m";
const char* const YELLOW = "\033[93m";
const char* const RED = "\033[91m";
const char* const CYAN = "\033[96m";
const char* const RESET = "\033[0m";

void enableVirtualTerminal() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode)) {
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

// Windows normally wakes sleeping threads only every ~15.6 ms, which would
// stretch every 10 ms tick to 15.6 ms. This asks for 1 ms resolution for this
// process (Windows undoes it when the process exits). winmm is loaded at
// runtime so building doesn't need an extra linker flag.
void requestFineTimer() {
    HMODULE winmm = LoadLibraryA("winmm.dll");
    if (!winmm) {
        return;
    }
    typedef UINT(WINAPI * TimeBeginPeriodFn)(UINT);
    FARPROC proc = GetProcAddress(winmm, "timeBeginPeriod");
    if (proc) {
        reinterpret_cast<TimeBeginPeriodFn>(reinterpret_cast<void*>(proc))(1);
    }
}

void clearScreen() {
    // Clear the screen and the scrollback, then move the cursor to the top.
    std::cout << "\033[2J\033[3J\033[H" << std::flush;
}

std::string timestamp() {
    SYSTEMTIME now;
    GetLocalTime(&now);
    int hour = now.wHour % 12;
    if (hour == 0) {
        hour = 12;
    }
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "%02d/%02d/%04d %02d:%02d:%02d%s", now.wMonth, now.wDay,
                  now.wYear, hour, now.wMinute, now.wSecond, now.wHour < 12 ? "AM" : "PM");
    return buffer;
}

}  // namespace console
