#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <conio.h>
#include <windows.h>

namespace {

const char* const GREEN = "\033[32m";
const char* const YELLOW = "\033[93m";
const char* const RESET = "\033[0m";
const std::chrono::milliseconds LINE_DELAY(60);

// The marquee switches to the next color every time it hits a wall, DVD-logo style.
const char* const MARQUEE_COLORS[] = {
    "\033[96m",  // cyan
    "\033[95m",  // magenta
    "\033[93m",  // yellow
    "\033[92m",  // green
    "\033[94m",  // blue
    "\033[91m",  // red
};
constexpr int MARQUEE_COLOR_COUNT = sizeof(MARQUEE_COLORS) / sizeof(MARQUEE_COLORS[0]);

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
constexpr int DEFAULT_SPEED_MS = 50;
const char* const DEFAULT_MARQUEE_TEXT = "Welcome to pe$OS!";

// The marquee box sits at the top of the screen; the rows under it are for the
// header and command output. The box takes whatever height is left after
// COMMAND_AREA_ROWS, within these limits.
constexpr int COMMAND_AREA_ROWS = 18;  // header (17 lines) + prompt
constexpr int MIN_BOX_ROWS = 9;
constexpr int MAX_BOX_ROWS = 16;

// Movement: the text moves exactly one column sideways every frame, so it
// visibly moves on every frame at a steady pace. Each bounce picks a new random
// angle between MIN and MAX degrees from horizontal, which sets how many rows
// it climbs or drops per column, so it never moves purely sideways or purely
// up/down. Console cells are about twice as tall as they are wide, so the
// vertical step is halved to make the angles look right on screen.
constexpr double MIN_ANGLE_DEG = 20.0;
constexpr double MAX_ANGLE_DEG = 45.0;
constexpr double CELL_ASPECT = 0.5;  // cell width / cell height
constexpr double PI = 3.14159265358979323846;

int clampInt(int value, int lo, int hi) {
    return std::max(lo, std::min(value, hi));
}

double clampDouble(double value, double lo, double hi) {
    return std::max(lo, std::min(value, hi));
}

// ---------------------------------------------------------------------------
// Screen layout, measured once at startup and read-only once the marquee
// thread is running.
// ---------------------------------------------------------------------------
struct Layout {
    int rows = 30;     // console window height
    int boxRows = 12;  // marquee box height, border included
    int boxCols = 79;  // marquee box width, border included
    int innerRows() const { return boxRows - 2; }
    int innerCols() const { return boxCols - 2; }
};

Layout layout;

// ---------------------------------------------------------------------------
// Shared marquee state. The display thread and the main thread both read and
// mutate this under `mtx`; `cv` lets command handlers wake the display thread
// immediately instead of waiting out the current frame's delay.
// ---------------------------------------------------------------------------
struct MarqueeState {
    std::mutex mtx;
    std::condition_variable cv;
    std::string text;
    double x = 0;   // position of the text's first character, relative to the box interior
    double y = 0;
    double vx = 0;  // distance moved per frame on each axis
    double vy = 0;
    int color = 0;  // index into MARQUEE_COLORS
    int speedMs = DEFAULT_SPEED_MS;
    bool running = false;
    bool dirty = false;  // text/speed/running changed; wake the thread early
    bool quit = false;
};

MarqueeState marquee;
std::mutex consoleMutex;  // guards every write to stdout so frames never interleave
std::atomic<bool> interrupted(false);  // set by Ctrl+C / Ctrl+Break

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

// Windows normally wakes sleeping threads only every ~15.6 ms, so a 50 ms
// frame really takes 47 or 62 ms and every set_speed under ~16 ms behaves the
// same. This asks for 1 ms timer resolution for this process (Windows undoes
// it when the process exits). winmm is loaded at runtime so building doesn't
// need an extra linker flag.
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

// Ctrl+C / Ctrl+Break would normally kill the program on the spot, leaving the
// console stuck with the marquee's scroll region and possibly a hidden cursor.
// Instead, flag it so the main loop shuts down the same way 'exit' does.
BOOL WINAPI onConsoleCtrl(DWORD type) {
    if (type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT) {
        interrupted = true;
        return TRUE;
    }
    return FALSE;
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

Layout measureLayout() {
    Layout l;
    l.rows = consoleRows();
    l.boxRows = clampInt(l.rows - COMMAND_AREA_ROWS, MIN_BOX_ROWS, MAX_BOX_ROWS);
    // Leave the last column unused: writing there can make some consoles wrap.
    l.boxCols = std::max(12, consoleCols() - 1);
    return l;
}

// The box rows are outside the scroll region, so command output scrolls
// underneath the box without ever moving it. Also puts the cursor under the box.
void reserveMarqueeBox() {
    std::cout << "\033[" << layout.boxRows + 1 << ';' << layout.rows << 'r'
              << "\033[" << layout.boxRows + 1 << ";1H" << std::flush;
}

void printSync(const std::string& s) {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << s << std::flush;
}

// ---------------------------------------------------------------------------
// Header
// ---------------------------------------------------------------------------
void printLine(const std::string& line, bool animate) {
    printSync(line + '\n');
    if (animate) {
        std::this_thread::sleep_for(LINE_DELAY);
    }
}

void printHeader(bool animate = false) {
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
}

// Clears only the command area under the marquee box.
void clearScreen() {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "\033[" << layout.boxRows + 1 << ";1H\033[0J" << std::flush;
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
    {"start_marquee", "Starts the bouncing marquee animation"},
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
// Marquee movement
// ---------------------------------------------------------------------------

// Furthest the text can go before touching the right/bottom wall.
double maxX() {
    return std::max(0, layout.innerCols() - static_cast<int>(marquee.text.size()));
}

double maxY() {
    return std::max(0, layout.innerRows() - 1);
}

// Sets the marquee text. Caller must hold marquee.mtx. Returns false when the
// text is wider than the box and will be cut off.
bool applyMarqueeText(const std::string& text) {
    marquee.text = text;
    // Pull the text back inside the box if it got longer.
    marquee.x = clampDouble(marquee.x, 0.0, maxX());
    marquee.y = clampDouble(marquee.y, 0.0, maxY());
    marquee.dirty = true;
    return static_cast<int>(text.size()) <= layout.innerCols();
}

// Points the marquee in a new random direction. signX/signY (+1 or -1) say
// which way it must travel on each axis, e.g. away from the wall it just hit.
// Caller must hold marquee.mtx.
void randomizeDirection(int signX, int signY) {
    static std::mt19937 rng(
        static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<double> degrees(MIN_ANGLE_DEG, MAX_ANGLE_DEG);
    const double angle = degrees(rng) * PI / 180.0;
    marquee.vx = signX * 1.0;
    marquee.vy = signY * std::tan(angle) * CELL_ASPECT;
}

// Moves the marquee one frame. On hitting a wall it bounces away from it at a
// new random angle; a corner hit bounces it back away from both walls.
// Caller must hold marquee.mtx.
void advanceMarquee() {
    marquee.x += marquee.vx;
    marquee.y += marquee.vy;

    int signX = marquee.vx < 0 ? -1 : 1;
    int signY = marquee.vy < 0 ? -1 : 1;
    bool bounced = false;
    // An axis with no room to move (text as wide as the box) never bounces.
    if (maxX() <= 0) {
        marquee.x = 0;
    } else if (marquee.x <= 0) {
        marquee.x = 0;
        signX = 1;
        bounced = true;
    } else if (marquee.x >= maxX()) {
        marquee.x = maxX();
        signX = -1;
        bounced = true;
    }
    if (maxY() <= 0) {
        marquee.y = 0;
    } else if (marquee.y <= 0) {
        marquee.y = 0;
        signY = 1;
        bounced = true;
    } else if (marquee.y >= maxY()) {
        marquee.y = maxY();
        signY = -1;
        bounced = true;
    }

    if (bounced) {
        randomizeDirection(signX, signY);
        marquee.color = (marquee.color + 1) % MARQUEE_COLOR_COUNT;
    }
}

// ---------------------------------------------------------------------------
// Marquee rendering (runs on its own thread)
// ---------------------------------------------------------------------------

// Lays out the box interior as one string per row, with the text at (x, y).
std::vector<std::string> layoutBoxRows(const std::string& text, int x, int y) {
    const int innerCols = layout.innerCols();
    std::vector<std::string> rows(layout.innerRows(), std::string(innerCols, ' '));
    if (y >= 0 && y < static_cast<int>(rows.size())) {
        for (int c = 0; c < static_cast<int>(text.size()) && x + c < innerCols; ++c) {
            rows[y][x + c] = text[c];
        }
    }
    return rows;
}

// Builds the escape codes that bring the box on screen up to date, as one
// string so each frame reaches the console in a single write. `onScreen`
// remembers what each interior row currently shows; rows that haven't changed
// are skipped, so a frame only redraws the line or two the text moved through.
// An empty `onScreen` means the box isn't drawn yet, so the border is drawn
// too. Rows are overwritten in full, so nothing is erased first and nothing
// flickers. The cursor is hidden while drawing and put back where the user
// was typing.
std::string drawBox(const std::vector<std::string>& rows, const char* color,
                    std::vector<std::string>& onScreen) {
    std::string out = "\033[?25l\0337";  // hide cursor, save its position
    if (onScreen.size() != rows.size()) {
        const std::string border = "+" + std::string(layout.innerCols(), '-') + "+";
        out += "\033[1;1H" + border;
        out += "\033[" + std::to_string(layout.boxRows) + ";1H" + border;
        onScreen.assign(rows.size(), "");
    }
    for (std::size_t r = 0; r < rows.size(); ++r) {
        // Blank rows don't need a color, so a color change doesn't redraw them.
        const bool blank = rows[r].find_first_not_of(' ') == std::string::npos;
        std::string shown = blank ? rows[r] : color + rows[r];
        if (shown != onScreen[r]) {
            out += "\033[" + std::to_string(r + 2) + ";1H|" + shown + RESET + "|";
            onScreen[r] = std::move(shown);
        }
    }
    out += "\0338\033[?25h";  // restore cursor position, show it again
    return out;
}

void marqueeLoop() {
    std::vector<std::string> onScreen;  // empty, so the first frame redraws the whole box
    bool textShown = false;             // main() draws the empty box before this thread starts
    bool advance = false;               // false = redraw in place (just started, or a command changed something)
    std::unique_lock<std::mutex> lock(marquee.mtx);
    while (!marquee.quit) {
        if (!marquee.running) {
            if (textShown) {
                const std::string frame = drawBox(layoutBoxRows("", 0, 0), RESET, onScreen);
                lock.unlock();
                printSync(frame);
                lock.lock();
                textShown = false;
            }
            marquee.cv.wait(lock, [] { return marquee.running || marquee.quit; });
            advance = false;
            continue;
        }

        // The next frame is due `speed` after this one starts, so the time
        // spent drawing doesn't slow the animation. If a frame runs late, the
        // next one still waits a full `speed` instead of rushing to catch up,
        // which would look like a sudden burst of speed.
        const auto nextFrame = std::chrono::steady_clock::now() + std::chrono::milliseconds(marquee.speedMs);
        if (advance) {
            advanceMarquee();
        }
        const std::vector<std::string> rows =
            layoutBoxRows(marquee.text, static_cast<int>(std::lround(marquee.x)),
                          static_cast<int>(std::lround(marquee.y)));
        const std::string frame = drawBox(rows, MARQUEE_COLORS[marquee.color], onScreen);
        marquee.dirty = false;
        lock.unlock();
        printSync(frame);
        lock.lock();
        textShown = true;

        // Timing out means it's time for the next step. Waking early means a
        // command changed something, so the next frame redraws without moving.
        advance = !marquee.cv.wait_until(lock, nextFrame,
                                         [] { return marquee.quit || !marquee.running || marquee.dirty; });
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
        if (interrupted) {
            printSync("\n");
            return "exit";
        }
        if (_kbhit()) {
            int ch = _getch();
            if (ch == '\r' || ch == '\n') {
                printSync("\n");
                return buffer;
            } else if (ch == 3) {  // Ctrl+C that arrives as a keystroke instead of a signal
                printSync("\n");
                return "exit";
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
    requestFineTimer();
    SetConsoleCtrlHandler(onConsoleCtrl, TRUE);
    layout = measureLayout();
    {
        std::lock_guard<std::mutex> lock(marquee.mtx);
        applyMarqueeText(DEFAULT_MARQUEE_TEXT);
        randomizeDirection(1, 1);  // starts in the top-left corner, so head down and right
    }

    std::vector<std::string> startupBox;
    std::cout << "\033[2J\033[H" << drawBox(layoutBoxRows("", 0, 0), RESET, startupBox);
    reserveMarqueeBox();
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
            bool wasRunning = false;
            {
                std::lock_guard<std::mutex> lock(marquee.mtx);
                wasRunning = marquee.running;
                marquee.running = true;
            }
            marquee.cv.notify_all();
            printSync(wasRunning ? "Marquee is already running.\n" : "Marquee started.\n");
        } else if (command == "stop_marquee") {
            bool wasRunning = false;
            {
                std::lock_guard<std::mutex> lock(marquee.mtx);
                wasRunning = marquee.running;
                marquee.running = false;
            }
            marquee.cv.notify_all();
            printSync(wasRunning ? "Marquee stopped.\n" : "Marquee is not running.\n");
        } else if (command == "set_text") {
            if (args.empty()) {
                printSync("Usage: set_text <text>\n");
            } else {
                bool fits = true;
                {
                    std::lock_guard<std::mutex> lock(marquee.mtx);
                    fits = applyMarqueeText(args);
                }
                marquee.cv.notify_all();
                printSync("Marquee text set to: " + args + "\n");
                if (!fits) {
                    printSync("That's wider than the marquee box, so the end will be cut off.\n");
                }
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
    // Restore full-screen scrolling, clear, and make sure the cursor is visible.
    std::cout << "\033[r\033[2J\033[H\033[?25h" << std::flush;
    return 0;
}
