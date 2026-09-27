#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <iomanip>
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
    printLine(std::string(YELLOW) + "Type 'help' to list commands, 'exit' to quit, 'clear' to clear the screen", animate);
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

struct CommandInfo {
    const char* name;
    const char* usage;
    const char* description;
};

const CommandInfo COMMANDS[] = {
    {"help", "help", "Displays the available commands and their descriptions"},
    {"start_marquee", "start_marquee", "Starts the marquee animation"},
    {"stop_marquee", "stop_marquee", "Stops the marquee animation"},
    {"set_text", "set_text <text>", "Sets the text shown in the marquee"},
    {"set_speed", "set_speed <ms>", "Sets the marquee refresh rate in milliseconds"},
    {"clear", "clear", "Clears the screen and reprints the header"},
    {"exit", "exit", "Terminates the console"},
};

const CommandInfo* findCommand(const std::string& name) {
    for (const CommandInfo& info : COMMANDS) {
        if (name == info.name) {
            return &info;
        }
    }
    return nullptr;
}

void printHelp() {
    std::cout << "Available commands:\n";
    for (const CommandInfo& info : COMMANDS) {
        std::cout << "  " << YELLOW << std::left << std::setw(18) << info.usage << RESET
                  << info.description << '\n';
    }
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

        const std::string line = trim(input);
        if (line.empty()) {
            continue;
        }

        // First word is the command; everything after it is its argument text.
        const std::size_t split = line.find_first_of(" \t");
        const std::string command = line.substr(0, split);
        const std::string args = split == std::string::npos ? "" : trim(line.substr(split));

        if (command == "exit") {
            break;
        } else if (command == "clear") {
            clearScreen();
            printHeader(true);
        } else if (command == "help") {
            printHelp();
        } else if (findCommand(command)) {
            std::cout << YELLOW << command << RESET << " command recognized. Doing something.";
            if (!args.empty()) {
                std::cout << " (args: " << args << ")";
            }
            std::cout << '\n';
        } else {
            std::cout << "Unknown command: " << command << ". Type 'help' to list commands.\n";
        }
    }
    return 0;
}
