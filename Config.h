#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <string>

enum class SchedulerType { FCFS, RR };

// Settings read from config.txt by the 'initialize' command.
struct Config {
    int numCpu = 0;                        // 1-128 CPU cores
    SchedulerType scheduler = SchedulerType::FCFS;
    std::uint64_t quantumCycles = 0;       // RR time slice, in CPU ticks
    std::uint64_t batchProcessFreq = 0;    // scheduler-start makes a process every this many ticks
    std::uint64_t minIns = 0;              // instructions per generated process
    std::uint64_t maxIns = 0;
    std::uint64_t delaysPerExec = 0;       // extra ticks a core stays busy after each instruction
};

// Reads the config file at `path` into `config`. On failure returns false and
// puts a message saying what's wrong in `error`.
bool loadConfig(const std::string& path, Config& config, std::string& error);

std::string schedulerName(SchedulerType type);

#endif
