#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

#include <conio.h>
#include <windows.h>

namespace {

const char* const GREEN = "\033[32m";
const char* const YELLOW = "\033[93m";
const char* const CYAN = "\033[96m";
const char* const RESET = "\033[0m";
const std::chrono::milliseconds LINE_DELAY(60);

const char* const VERSION_DATE = "2026-09-27";
const char* const GROUP_MEMBERS[] = {
    "Kimberly Moraca Chong",
    "Edlynn Rei Encallado",
    "Michael Stephen Maglente",
    "Miguel Angelo Ignacio"};

// How often the input loop checks the keyboard, in milliseconds. Tunable to
// measure the refresh-rate/polling-rate tradeoff (screen tearing vs. typing
// delay) the spec asks about.
constexpr int POLL_MS = 15;
constexpr int DEFAULT_SPEED_MS = 200;
const char* const DEFAULT_MARQUEE_TEXT = "Welcome to pe$OS!";

// ---------------------------------------------------------------------------
// Shared marquee state. The display thread and the main thread both read and
// mutate this under `mtx`; `cv` lets command handlers wake the display thread
// immediately instead of waiting out the current frame's delay.
// ---------------------------------------------------------------------------
struct MarqueeState {
    std::mutex mtx;
    std::condition_variable cv;
    std::string text = DEFAULT_MARQUEE_TEXT;
    int speedMs = DEFAULT_SPEED_MS;
    std::size_t offset = 0;
    bool running = false;
    bool dirty = false;  // text/speed/running changed; wake the thread early
    bool quit = false;
};

MarqueeState marquee;
std::mutex consoleMutex;  // guards every write to stdout so frames never interleave

// ---------------------------------------------------------------------------
// Console helpers
// ---------------------------------------------------------------------------
void enableVirtualTerminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode)) {
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

int consoleRows() {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        return info.srWindow.Bottom - info.srWindow.Top + 1;
    }
    return 30;
}

int consoleCols() {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        return info.srWindow.Right - info.srWindow.Left + 1;
    }
    return 80;
}

// Row 1 is reserved for the marquee for the program's whole lifetime; every
// other command (including startup and 'clear') prints starting at row 2.
void reserveMarqueeRow() {
    std::cout << "\033[2;" << consoleRows() << "r";  // scroll region = rows 2..bottom
}

void releaseMarqueeRow() {
    std::cout << "\033[r";  // restore full-screen scrolling before exit
}

void printSync(const std::string& s) {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << s << std::flush;
}

// ---------------------------------------------------------------------------
// Header
// ---------------------------------------------------------------------------
void printLine(const std::string& line, bool animate) {
    {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << line << '\n' << std::flush;
    }
    if (animate) {
        std::this_thread::sleep_for(LINE_DELAY);
    }
}

void printHeader(bool animate = false) {
    printLine("", animate);
    printLine(R"ASCII(                _     ___   ____)ASCII", animate);
    printLine(R"ASCII( _ __    ___   | |   / _ \ / ___|)ASCII", animate);
    printLine(R"ASCII(| '_ \  / _ \ / __) | | | |\___ \)ASCII", animate);
    printLine(R"ASCII(| |_) ||  __/ \__ \ | |_| | ___) |)ASCII", animate);
    printLine(R"ASCII(| .__/  \___| (   /  \___/ |____/)ASCII", animate);
    printLine(R"ASCII(|_|            |_|)ASCII", animate);
    printLine(std::string(GREEN) + "Hi there! Welcome to pe$OS commandline!" + RESET, animate);
    printLine("", animate);
    printLine("Group developers:", animate);
    for (const char* const member : GROUP_MEMBERS) {
        printLine(member, animate);
    }
    printLine("", animate);
    printLine(std::string("Version date: ") + VERSION_DATE, animate);
    printLine("", animate);
    printLine(std::string(YELLOW) + "Type 'help' to list commands, 'exit' to quit, 'clear' to clear the screen" + RESET, animate);
    printLine("", animate);
}

// Row 1 stays reserved for the marquee; this clears from row 2 down only.
void clearScreen() {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "\033[2;1H\033[0J" << std::flush;
}

std::string trim(const std::string& s) {
    auto first = std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
    auto last = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return first < last ? std::string(first, last) : std::string();
}

// ---------------------------------------------------------------------------
// Command table (drives 'help')
// ---------------------------------------------------------------------------
struct CommandInfo {
    const char* usage;
    const char* description;
};

const CommandInfo COMMANDS[] = {
    {"help", "Displays the available commands and their descriptions"},
    {"start_marquee", "Starts the marquee animation"},
    {"stop_marquee", "Stops the marquee animation"},
    {"set_text <text>", "Sets the text shown in the marquee"},
    {"set_speed <ms>", "Sets the marquee refresh rate in milliseconds"},
    {"clear", "Clears the screen and reprints the header"},
    {"exit", "Terminates the console"},
};

void printHelp() {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "Available commands:\n";
    for (const CommandInfo& info : COMMANDS) {
        std::cout << "  " << YELLOW << std::left << std::setw(18) << info.usage << RESET
                  << info.description << '\n';
    }
    std::cout << std::flush;
}

// ---------------------------------------------------------------------------
// Marquee rendering (runs on its own thread)
// ---------------------------------------------------------------------------

// Builds the next `width`-wide slice of the scrolling text and advances
// `offset` by one column. Pure aside from the offset it's handed.
std::string buildMarqueeFrame(const std::string& text, std::size_t width, std::size_t& offset) {
    if (text.empty()) {
        return std::string(width, ' ');
    }
    const std::string content = text + "   ";
    std::string ring;
    while (ring.size() < width + content.size()) {
        ring += content;
    }
    offset %= content.size();
    std::string frame = ring.substr(offset, width);
    offset = (offset + 1) % content.size();
    return frame;
}

void drawMarqueeRow(const std::string& frame) {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "\0337"        // save cursor position (DECSC)
              << "\033[1;1H"    // jump to row 1
              << "\033[2K"      // clear the row
              << CYAN << frame << RESET
              << "\0338"        // restore cursor position (DECRC)
              << std::flush;
}

void blankMarqueeRow() {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "\0337" << "\033[1;1H" << "\033[2K" << "\0338" << std::flush;
}

void marqueeLoop() {
    bool blanked = true;  // row 1 starts blank (full-screen clear happens before this runs)
    std::unique_lock<std::mutex> lock(marquee.mtx);
    while (!marquee.quit) {
        if (!marquee.running) {
            if (!blanked) {
                lock.unlock();
                blankMarqueeRow();
                lock.lock();
                blanked = true;
            }
            marquee.cv.wait(lock, [] { return marquee.running || marquee.quit; });
            continue;
        }

        blanked = false;
        const int width = std::max(10, consoleCols() - 2);
        const std::string frame = buildMarqueeFrame(marquee.text, static_cast<std::size_t>(width), marquee.offset);
        const int speed = marquee.speedMs;
        lock.unlock();
        drawMarqueeRow(frame);
        lock.lock();

        marquee.cv.wait_for(lock, std::chrono::milliseconds(speed),
                             [] { return marquee.quit || !marquee.running || marquee.dirty; });
        marquee.dirty = false;
    }
}

// ---------------------------------------------------------------------------
// Input: manual polling loop instead of std::cin >> / getline, so the
// refresh rate (marquee) and the polling rate (keyboard) are both explicit,
// tunable values, per the assignment's request to characterize that tradeoff.
// ---------------------------------------------------------------------------
std::string readCommandLine() {
    std::string buffer;
    while (true) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == '\r' || ch == '\n') {
                printSync("\n");
                return buffer;
            } else if (ch == 8) {  // backspace
                if (!buffer.empty()) {
                    buffer.pop_back();
                    printSync("\b \b");
                }
            } else if (ch == 0 || ch == 224) {
                _getch();  // discard the second byte of an extended key (arrows, F-keys, ...)
            } else if (ch >= 32 && ch < 127) {
                buffer.push_back(static_cast<char>(ch));
                printSync(std::string(1, static_cast<char>(ch)));
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(POLL_MS));
        }
    }
}

}  // namespace

int main() {
    enableVirtualTerminal();
    std::cout << "\033[2J\033[H" << std::flush;  // clear everything once, cursor home
    reserveMarqueeRow();
    std::cout << "\033[2;1H" << std::flush;  // header starts at row 2, row 1 stays reserved

    printHeader();

    std::thread marqueeThread(marqueeLoop);

    while (true) {
        printSync("Command> ");
        const std::string line = trim(readCommandLine());
        if (line.empty()) {
            continue;
        }

        // First word is the command; everything after it is its argument text.
        const std::size_t split = line.find_first_of(" \t");
        const std::string command = line.substr(0, split);
        const std::string args = split == std::string::npos ? "" : trim(line.substr(split));

        if (command == "exit") {
            {
                std::lock_guard<std::mutex> lock(marquee.mtx);
                marquee.quit = true;
            }
            marquee.cv.notify_all();
            break;
        } else if (command == "clear") {
            clearScreen();
            printHeader(true);
        } else if (command == "help") {
            printHelp();
        } else if (command == "start_marquee") {
            {
                std::lock_guard<std::mutex> lock(marquee.mtx);
                marquee.running = true;
                marquee.dirty = true;
            }
            marquee.cv.notify_all();
            printSync("Marquee started.\n");
        } else if (command == "stop_marquee") {
            {
                std::lock_guard<std::mutex> lock(marquee.mtx);
                marquee.running = false;
                marquee.dirty = true;
            }
            marquee.cv.notify_all();
            printSync("Marquee stopped.\n");
        } else if (command == "set_text") {
            if (args.empty()) {
                printSync("Usage: set_text <text>\n");
            } else {
                {
                    std::lock_guard<std::mutex> lock(marquee.mtx);
                    marquee.text = args;
                    marquee.offset = 0;
                    marquee.dirty = true;
                }
                marquee.cv.notify_all();
                printSync("Marquee text set to: " + args + "\n");
            }
        } else if (command == "set_speed") {
            bool ok = !args.empty() &&
                      std::all_of(args.begin(), args.end(), [](unsigned char c) { return std::isdigit(c); });
            int value = 0;
            if (ok) {
                try {
                    value = std::stoi(args);
                    ok = value > 0;
                } catch (const std::exception&) {
                    ok = false;
                }
            }
            if (!ok) {
                printSync("Usage: set_speed <positive integer milliseconds>\n");
            } else {
                {
                    std::lock_guard<std::mutex> lock(marquee.mtx);
                    marquee.speedMs = value;
                    marquee.dirty = true;
                }
                marquee.cv.notify_all();
                printSync("Marquee speed set to " + std::to_string(value) + " ms.\n");
            }
        } else {
            printSync("Unknown command: " + command + ". Type 'help' to list commands.\n");
        }
    }

    marqueeThread.join();
    releaseMarqueeRow();
    std::cout << "\033[2J\033[H" << std::flush;
    return 0;
}
