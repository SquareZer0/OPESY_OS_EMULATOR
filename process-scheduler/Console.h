#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>

// Small helpers for talking to the Windows console.
namespace console {

extern const char* const GREEN;
extern const char* const YELLOW;
extern const char* const RED;
extern const char* const CYAN;
extern const char* const RESET;

// Turns on ANSI escape codes (colors, clearing the screen) for this console.
void enableVirtualTerminal();

// Asks Windows for 1 ms timer resolution so the 10 ms CPU tick stays accurate.
void requestFineTimer();

void clearScreen();

// The current local time as "MM/DD/YYYY hh:mm:ssAM".
std::string timestamp();

}  // namespace console

#endif
