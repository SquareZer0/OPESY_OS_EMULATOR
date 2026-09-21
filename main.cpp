#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

namespace {

const char* const GREEN = "\033[32m";
const char* const YELLOW = "\033[93m";
const char* const RESET = "\033[0m";

void printHeader() {
    std::cout << R"ASCII(
                _     ___   ____
 _ __    ___   | |   / _ \ / ___|
| '_ \  / _ \ / __) | | | |\___ \
| |_) ||  __/ \__ \ | |_| | ___) |
| .__/  \___| (   /  \___/ |____/
|_|            |_|
)ASCII";
    std::cout << GREEN << "Hi there! Welcome to pe$OS commandline!\n" << RESET;
    std::cout << YELLOW << "Type 'exit' to quit, 'clear' to clear the screen\n\n";
    std::cout << "** IMPORTANT: Type 'initialize' to load config and start system **\n"
              << RESET << '\n';
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
        std::cout << "Enter a command: ";
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
            printHeader();
        } else if (isRecognized(command)) {
            std::cout << command << " command recognized. Doing something.\n";
        } else {
            std::cout << "Unknown command: " << command << '\n';
        }
    }
    return 0;
}
