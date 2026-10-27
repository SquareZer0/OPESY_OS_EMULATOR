// pe$OS - CSOPESY process scheduler emulator. Entry point (main) is in this file.

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX  // stop windows.h from defining min/max macros that clash with std::min/std::max
#endif
#include <windows.h>

#include "Config.h"
#include "Console.h"
#include "Scheduler.h"

namespace {

const char* const VERSION_DATE = "2026-10-27";
const char* const GROUP_MEMBERS[] = {
    "Kimberly Moraca Chong",
    "Edlynn Rei Encallado",
    "Michael Stephen Maglente",
    "Miguel Angelo Ignacio",
};
const char* const CONFIG_FILE = "config.txt";
const char* const REPORT_FILE = "csopesy-log.txt";

void printHeader(bool initialized) {
    std::cout << "                _     ___   ____\n"
                 " _ __    ___   | |   / _ \\ / ___|\n"
                 "| '_ \\  / _ \\ / __) | | | |\\___ \\\n"
                 "| |_) ||  __/ \\__ \\ | |_| | ___) |\n"
                 "| .__/  \\___| (   /  \\___/ |____/\n"
                 "|_|            |_|\n";
    std::cout << console::GREEN << "Hi there! Welcome to pe$OS commandline!" << console::RESET << "\n\n";
    std::cout << "Group developers:\n";
    for (std::size_t i = 0; i < sizeof GROUP_MEMBERS / sizeof GROUP_MEMBERS[0]; ++i) {
        std::cout << GROUP_MEMBERS[i] << '\n';
    }
    std::cout << "\nVersion date: " << VERSION_DATE << "\n\n";
    if (initialized) {
        std::cout << console::YELLOW << "Type 'help' to list commands, 'exit' to quit." << console::RESET << "\n\n";
    } else {
        std::cout << console::YELLOW << "** IMPORTANT: Type 'initialize' to load " << CONFIG_FILE
                  << " and start the system, or 'exit' to quit. **" << console::RESET << "\n\n";
    }
}

std::vector<std::string> splitWords(const std::string& line) {
    std::istringstream in(line);
    std::vector<std::string> words;
    std::string word;
    while (in >> word) {
        words.push_back(word);
    }
    return words;
}

void printError(const std::string& message) {
    std::cout << console::RED << message << console::RESET << '\n';
}

std::string currentDirectory() {
    char buffer[MAX_PATH];
    const DWORD length = GetCurrentDirectoryA(MAX_PATH, buffer);
    return length > 0 && length < MAX_PATH ? std::string(buffer) : std::string(".");
}

// The commands that exist once the system is initialized, for 'help' and for
// telling "not initialized yet" apart from "unknown command".
bool isMainMenuCommand(const std::string& command) {
    const char* const commands[] = {"screen", "scheduler-start", "scheduler-stop", "report-util", "help", "clear"};
    for (std::size_t i = 0; i < sizeof commands / sizeof commands[0]; ++i) {
        if (command == commands[i]) {
            return true;
        }
    }
    return false;
}

void printHelp() {
    struct Entry {
        const char* usage;
        const char* description;
    };
    const Entry entries[] = {
        {"initialize", "Loads config.txt and starts the CPU cores (already done)"},
        {"screen -s <name>", "Creates a process and opens its screen"},
        {"screen -r <name>", "Opens the screen of a process that hasn't finished"},
        {"screen -ls", "Shows CPU utilization, running and finished processes"},
        {"scheduler-start", "Starts creating dummy processes every batch-process-freq ticks"},
        {"scheduler-stop", "Stops creating dummy processes (existing ones keep running)"},
        {"report-util", "Saves the screen -ls output to csopesy-log.txt"},
        {"clear", "Clears the screen"},
        {"exit", "Shuts down the emulator"},
    };
    std::cout << "Available commands:\n";
    for (std::size_t i = 0; i < sizeof entries / sizeof entries[0]; ++i) {
        std::cout << "  " << console::YELLOW << std::left << std::setw(20) << entries[i].usage << console::RESET
                  << entries[i].description << '\n';
    }
}

void printConfig(const Config& config) {
    std::cout << "Loaded " << CONFIG_FILE << ":\n"
              << "  num-cpu             " << config.numCpu << '\n'
              << "  scheduler           " << schedulerName(config.scheduler) << '\n'
              << "  quantum-cycles      " << config.quantumCycles << '\n'
              << "  batch-process-freq  " << config.batchProcessFreq << '\n'
              << "  min-ins             " << config.minIns << '\n'
              << "  max-ins             " << config.maxIns << '\n'
              << "  delay-per-exec      " << config.delayPerExec << '\n';
}

void printProcessScreenHeader(const Process& process) {
    std::cout << "Process name: " << process.name() << '\n'
              << "ID: " << process.id() << '\n'
              << "Created: " << process.createdAt() << '\n'
              << "Current instruction line: " << process.executedInstructions() << " / "
              << process.totalInstructions() << "\n\n"
              << console::YELLOW << "Type 'process-smi' for details, 'exit' to return to the main menu."
              << console::RESET << "\n\n";
}

// The screen of one process. Returns false if input ended (the whole program
// should exit), true when the user typed 'exit' to go back to the main menu.
bool runProcessScreen(const std::shared_ptr<Process>& process) {
    console::clearScreen();
    printProcessScreenHeader(*process);
    std::string line;
    while (true) {
        std::cout << process->name() << ":\\> " << std::flush;
        if (!std::getline(std::cin, line)) {
            return false;
        }
        const std::vector<std::string> words = splitWords(line);
        if (words.empty()) {
            continue;
        }
        if (words[0] == "exit") {
            return true;
        } else if (words[0] == "process-smi") {
            std::cout << '\n' << process->smiReport() << '\n';
        } else {
            printError("Unknown command: " + words[0] + ". In a process screen you can use 'process-smi' or 'exit'.");
        }
    }
}

// Handles screen -s / -r / -ls. Returns false if the program should exit.
bool handleScreen(const std::vector<std::string>& words, Scheduler& scheduler) {
    const std::string option = words.size() > 1 ? words[1] : "";
    if (option == "-ls" && words.size() == 2) {
        std::cout << scheduler.utilizationReport();
        return true;
    }
    if ((option == "-s" || option == "-r") && words.size() == 3) {
        const std::string& name = words[2];
        std::shared_ptr<Process> process;
        if (option == "-s") {
            process = scheduler.createProcess(name);
            if (!process) {
                printError("Process " + name + " already exists. Use 'screen -r " + name + "' to open it.");
                return true;
            }
        } else {
            process = scheduler.findUnfinishedProcess(name);
            if (!process) {
                printError("Process " + name + " not found.");
                return true;
            }
        }
        if (!runProcessScreen(process)) {
            return false;
        }
        console::clearScreen();
        printHeader(true);
        return true;
    }
    printError("Usage: screen -s <process_name> | screen -r <process_name> | screen -ls");
    return true;
}

}  // namespace

int main() {
    console::enableVirtualTerminal();
    console::requestFineTimer();
    console::clearScreen();
    printHeader(false);

    std::unique_ptr<Scheduler> scheduler;  // created by 'initialize'
    std::string line;
    while (true) {
        std::cout << "Command> " << std::flush;
        if (!std::getline(std::cin, line)) {
            break;
        }
        const std::vector<std::string> words = splitWords(line);
        if (words.empty()) {
            continue;
        }
        const std::string& command = words[0];

        if (command == "exit") {
            break;
        }

        if (command == "initialize") {
            if (scheduler) {
                std::cout << "The system is already initialized.\n";
                continue;
            }
            Config config;
            std::string error;
            if (!loadConfig(CONFIG_FILE, config, error)) {
                printError("Initialization failed: " + error);
                std::cout << "(Looked in " << currentDirectory() << ")\n";
                continue;
            }
            scheduler.reset(new Scheduler(config));
            scheduler->start();
            printConfig(config);
            std::cout << console::GREEN << "System initialized. The CPU cores are running; type 'help' to list commands."
                      << console::RESET << '\n';
            continue;
        }

        // Before 'initialize', only 'initialize' and 'exit' are allowed.
        if (!scheduler) {
            if (isMainMenuCommand(command)) {
                printError("'" + command + "' isn't available yet. Type 'initialize' first.");
            } else {
                printError("Unknown command: " + command + ". Type 'initialize' first.");
            }
            continue;
        }

        if (command == "screen") {
            if (!handleScreen(words, *scheduler)) {
                break;
            }
        } else if (command == "scheduler-start") {
            if (scheduler->startGenerating()) {
                std::cout << "Started generating a dummy process every " << scheduler->config().batchProcessFreq
                          << " CPU tick(s).\n";
            } else {
                std::cout << "Process generation is already running.\n";
            }
        } else if (command == "scheduler-stop") {
            if (scheduler->stopGenerating()) {
                std::cout << "Stopped generating new processes. Existing processes keep running.\n";
            } else {
                std::cout << "Process generation isn't running.\n";
            }
        } else if (command == "report-util") {
            std::ofstream out(REPORT_FILE);
            if (!out) {
                printError(std::string("Could not write ") + REPORT_FILE + ".");
                continue;
            }
            out << "Report generated: " << console::timestamp() << "\n\n" << scheduler->utilizationReport();
            std::cout << "Report generated at " << currentDirectory() << '\\' << REPORT_FILE << "!\n";
        } else if (command == "help") {
            printHelp();
        } else if (command == "clear") {
            console::clearScreen();
            printHeader(true);
        } else {
            printError("Unknown command: " + command + ". Type 'help' to list commands.");
        }
    }

    std::cout << "Shutting down...\n";
    if (scheduler) {
        scheduler->stop();
    }
    return 0;
}
