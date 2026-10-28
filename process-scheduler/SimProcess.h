#ifndef SIM_PROCESS_H
#define SIM_PROCESS_H

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "Instruction.h"

enum class ProcessState { Ready, Running, Sleeping, Finished };

// What happened when a core ran one instruction of a process.
struct StepResult {
    enum Kind { Continue, Sleep, Finished };
    Kind kind;
    std::uint8_t sleepTicks;  // for Sleep
};

// A simulated process: its program, where it is in that program, its
// variables and its log. Program state is guarded by the process's own mutex,
// so a core can run it while the CLI reads its progress and logs.
class Process {
public:
    Process(int id, const std::string& name, std::vector<Instruction> program);

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& createdAt() const { return createdAt_; }
    std::uint64_t totalInstructions() const { return total_; }

    // Runs the next instruction on core `coreId`. Only the core currently
    // holding this process calls it.
    StepResult executeNext(int coreId);

    std::uint64_t executedInstructions() const;
    bool isFinished() const;

    // The process-smi output: name, ID, logs, and progress or "Finished!".
    std::string smiReport() const;

    // Scheduler bookkeeping, guarded by the scheduler's mutex (not this one).
    ProcessState state = ProcessState::Ready;
    int core = -1;
    std::uint64_t delayLeft = 0;  // delay-per-exec ticks still owed when it was taken off its core

private:
    // Where execution is inside one instruction list. FOR pushes a frame for
    // its body; the bottom frame is the whole program, run once.
    struct Frame {
        const std::vector<Instruction>* list;
        std::size_t pc;
        std::uint64_t iterationsLeft;
    };

    // Both expect mtx_ to be held.
    std::uint16_t read(const Operand& operand);
    void run(const Instruction& instruction, int coreId, StepResult& result);

    const int id_;
    const std::string name_;
    const std::string createdAt_;
    const std::vector<Instruction> program_;
    const std::uint64_t total_;

    mutable std::mutex mtx_;
    std::uint64_t executed_ = 0;
    std::vector<Frame> stack_;
    std::map<std::string, std::uint16_t> variables_;
    std::vector<std::string> logs_;
};

#endif
