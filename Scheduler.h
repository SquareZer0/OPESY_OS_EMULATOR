#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "Config.h"
#include "SimProcess.h"

// Runs processes on simulated CPU cores, from 'initialize' until the program exits.
//
// A clock thread drives time in ticks. Each tick it wakes sleeping processes,
// makes a new process if scheduler-start is on and batch-process-freq ticks
// have passed, hands ready processes to idle cores, and then lets every busy
// core's worker thread run one tick in parallel. Once all of them are done it
// applies the results: finished processes are recorded, sleeping ones are
// parked, and under RR a process that has used up its quantum goes to the back
// of the ready queue.
class Scheduler {
public:
    explicit Scheduler(const Config& config);
    ~Scheduler();

    void start();
    void stop();

    // Creates a process with a random program and queues it. Returns null if
    // the name is already taken.
    std::shared_ptr<Process> createProcess(const std::string& name);

    // The process with this name, unless it doesn't exist or has finished.
    std::shared_ptr<Process> findUnfinishedProcess(const std::string& name);

    // Turn dummy process generation on/off. Return false if it was already in that state.
    bool startGenerating();
    bool stopGenerating();

    // The screen -ls / report-util text: CPU utilization, cores, running and finished processes.
    std::string utilizationReport();

    const Config& config() const { return config_; }

private:
    struct Core {
        Core() : runTick(0), delayLeft(0), quantumUsed(0) {
            outcome.kind = StepResult::Continue;
            outcome.sleepTicks = 0;
        }
        std::condition_variable wake;
        std::shared_ptr<Process> process;  // null when idle
        std::uint64_t runTick;             // the tick this core was last told to run
        std::uint64_t delayLeft;           // busy ticks left from delays-per-exec
        std::uint64_t quantumUsed;         // ticks spent on the current process
        StepResult outcome;                // result of the core's last tick
    };

    struct Sleeper {
        std::shared_ptr<Process> process;
        std::uint64_t wakeTick;
    };

    void clockLoop();
    void coreLoop(int id);
    void runTick(std::unique_lock<std::mutex>& lock);  // lock holds mtx_

    std::vector<Instruction> randomProgram();
    // These expect mtx_ to be held.
    void dispatchLocked();
    std::shared_ptr<Process> addProcessLocked(const std::string& name, std::vector<Instruction> program);
    std::string nextAutoNameLocked();

    const Config config_;

    std::mutex mtx_;  // guards everything below
    std::condition_variable coresDone_;
    std::vector<std::unique_ptr<Core>> cores_;
    std::deque<std::shared_ptr<Process>> ready_;
    std::vector<Sleeper> sleepers_;
    std::map<std::string, std::shared_ptr<Process>> processes_;
    std::vector<std::shared_ptr<Process>> finished_;
    std::uint64_t tick_ = 0;
    int busyCores_ = 0;      // cores running a process this tick
    int coresFinished_ = 0;  // how many of them have finished the tick
    bool generating_ = false;
    std::uint64_t ticksSinceBatch_ = 0;
    int nextId_ = 1;
    int nextAutoNumber_ = 1;
    bool stopping_ = false;

    std::mutex rngMtx_;
    std::mt19937 rng_;

    std::thread clockThread_;
    std::vector<std::thread> coreThreads_;
};

#endif
