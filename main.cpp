#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace {

const char* const GREEN = "\033[32m";
const char* const YELLOW = "\033[93m";
const char* const RESET = "\033[0m";
const std::chrono::milliseconds LINE_DELAY(60);

const char* const VERSION_DATE = "2026-09-27";
const char* const GROUP_MEMBERS[] = {
    "Kimberly Moraca Chong",
    "Edlynn Rei Encallado",
    "Michael Stephen Maglente",
    "Miguel Angelo Ignacio"};

void printLine(const std::string& line, bool animate) {
    std::cout << line << '\n' << std::flush;
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
    printLine(std::string(YELLOW) + "Type 'exit' to quit, 'clear' to clear the screen", animate);
    printLine("", animate);
    printLine("** IMPORTANT: Type 'initialize' to load config and start system **", animate);
    printLine(RESET, animate);
}

void clearScreen() {
    std::cout << "\033[2J\033[H";
}

std::string trim(const std::string& s) {
    auto first = std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
    auto last = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return first < last ? std::string(first, last) : std::string();
}

bool isRecognized(const std::string& command) {
    static const char* const commands[] = {
        "initialize", "screen", "scheduler-start", "scheduler-stop", "report-util"};
    return std::any_of(std::begin(commands), std::end(commands),
                       [&](const char* c) { return command == c; });
}

}  // namespace

int main() {
    clearScreen();
    printHeader();

    std::string input;
    while (true) {
        std::cout << "Command> ";
        if (!std::getline(std::cin, input)) {
            break;  // EOF (e.g. Ctrl+Z / Ctrl+D)
        }

        const std::string command = trim(input);
        if (command.empty()) {
            continue;
        }

        if (command == "exit") {
            break;
        } else if (command == "clear") {
            clearScreen();
            printHeader(true);
        } else if (isRecognized(command)) {
            std::cout << YELLOW << command << RESET << " command recognized. Doing something.\n";
        } else {
            std::cout << "Unknown command: " << command << '\n';
        }
    }
    return 0;
}
