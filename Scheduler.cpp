#include "Scheduler.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace {

// Real time per CPU tick. batch-process-freq, delays-per-exec, quantum-cycles
// and SLEEP all count in ticks.
const std::chrono::milliseconds TICK_LENGTH(10);

}  // namespace

Scheduler::Scheduler(const Config& config)
    : config_(config),
      rng_(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count())) {}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    for (int i = 0; i < config_.numCpu; ++i) {
        cores_.push_back(std::unique_ptr<Core>(new Core()));
    }
    for (int i = 0; i < config_.numCpu; ++i) {
        coreThreads_.push_back(std::thread(&Scheduler::coreLoop, this, i));
    }
    clockThread_ = std::thread(&Scheduler::clockLoop, this);
}

void Scheduler::stop() {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        stopping_ = true;
        for (std::size_t i = 0; i < cores_.size(); ++i) {
            cores_[i]->wake.notify_one();
        }
        coresDone_.notify_all();
    }
    if (clockThread_.joinable()) {
        clockThread_.join();
    }
    for (std::size_t i = 0; i < coreThreads_.size(); ++i) {
        if (coreThreads_[i].joinable()) {
            coreThreads_[i].join();
        }
    }
}

void Scheduler::clockLoop() {
    std::chrono::steady_clock::time_point nextTick = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lock(mtx_);
    while (!stopping_) {
        lock.unlock();
        nextTick += TICK_LENGTH;
        std::this_thread::sleep_until(nextTick);
        // If the clock fell far behind (e.g. paused in a debugger), carry on
        // from now instead of racing through the missed ticks.
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if (now - nextTick > 10 * TICK_LENGTH) {
            nextTick = now;
        }
        lock.lock();
        if (!stopping_) {
            runTick(lock);
        }
    }
}

void Scheduler::runTick(std::unique_lock<std::mutex>& lock) {
    ++tick_;

    // Processes whose SLEEP is over go to the back of the ready queue.
    std::vector<Sleeper> stillSleeping;
    for (std::size_t i = 0; i < sleepers_.size(); ++i) {
        if (sleepers_[i].wakeTick <= tick_) {
            sleepers_[i].process->state = ProcessState::Ready;
            ready_.push_back(sleepers_[i].process);
        } else {
            stillSleeping.push_back(sleepers_[i]);
        }
    }
    sleepers_.swap(stillSleeping);

    // scheduler-start: one new process every batch-process-freq ticks.
    if (generating_ && ++ticksSinceBatch_ >= config_.batchProcessFreq) {
        ticksSinceBatch_ = 0;
        const std::string name = nextAutoNameLocked();
        addProcessLocked(name, randomProgram());
    }

    dispatchLocked();

    // Let every busy core run this tick, then wait for all of them.
    busyCores_ = 0;
    coresFinished_ = 0;
    for (std::size_t i = 0; i < cores_.size(); ++i) {
        if (cores_[i]->process) {
            cores_[i]->runTick = tick_;
            ++busyCores_;
            cores_[i]->wake.notify_one();
        }
    }
    coresDone_.wait(lock, [this] { return coresFinished_ == busyCores_ || stopping_; });
    if (stopping_) {
        return;
    }

    // Apply what each core's tick did.
    for (std::size_t i = 0; i < cores_.size(); ++i) {
        Core& core = *cores_[i];
        if (!core.process) {
            continue;
        }
        if (core.outcome.kind == StepResult::Finished) {
            core.process->state = ProcessState::Finished;
            core.process->core = -1;
            finished_.push_back(core.process);
            core.process.reset();
        } else if (core.outcome.kind == StepResult::Sleep) {
            // SLEEP(X): off the core for the next X ticks.
            core.process->state = ProcessState::Sleeping;
            core.process->core = -1;
            Sleeper sleeper = {core.process, tick_ + core.outcome.sleepTicks + 1};
            sleepers_.push_back(sleeper);
            core.process.reset();
        } else if (config_.scheduler == SchedulerType::RR && core.quantumUsed >= config_.quantumCycles) {
            // Round robin: quantum used up, back of the ready queue.
            core.process->state = ProcessState::Ready;
            core.process->core = -1;
            ready_.push_back(core.process);
            core.process.reset();
        }
    }

    // Refill cores freed this tick right away, so between ticks no core sits
    // idle while processes are waiting.
    dispatchLocked();
}

void Scheduler::dispatchLocked() {
    // Hand ready processes to idle cores, in queue order.
    for (std::size_t i = 0; i < cores_.size() && !ready_.empty(); ++i) {
        Core& core = *cores_[i];
        if (core.process) {
            continue;
        }
        core.process = ready_.front();
        ready_.pop_front();
        core.process->state = ProcessState::Running;
        core.process->core = static_cast<int>(i);
        core.delayLeft = 0;
        core.quantumUsed = 0;
    }
}

void Scheduler::coreLoop(int id) {
    Core& core = *cores_[id];
    std::uint64_t lastTick = 0;
    std::unique_lock<std::mutex> lock(mtx_);
    while (true) {
        core.wake.wait(lock, [&] { return stopping_ || core.runTick > lastTick; });
        if (stopping_) {
            return;
        }
        lastTick = core.runTick;
        std::shared_ptr<Process> process = core.process;
        lock.unlock();

        // Run one tick. The clock thread doesn't touch this core until all
        // busy cores report back, so its fields are safe to use unlocked here.
        StepResult outcome = {StepResult::Continue, 0};
        if (core.delayLeft > 0) {
            --core.delayLeft;  // still busy from delays-per-exec
        } else {
            outcome = process->executeNext(id);
            if (outcome.kind == StepResult::Continue) {
                core.delayLeft = config_.delaysPerExec;
            }
        }
        ++core.quantumUsed;

        lock.lock();
        core.outcome = outcome;
        if (++coresFinished_ == busyCores_) {
            coresDone_.notify_one();
        }
    }
}

std::vector<Instruction> Scheduler::randomProgram() {
    std::lock_guard<std::mutex> lock(rngMtx_);
    std::uniform_int_distribution<std::uint64_t> length(config_.minIns, config_.maxIns);
    return generateProgram(length(rng_), rng_);
}

std::shared_ptr<Process> Scheduler::addProcessLocked(const std::string& name, std::vector<Instruction> program) {
    std::shared_ptr<Process> process = std::make_shared<Process>(nextId_++, name, std::move(program));
    processes_[name] = process;
    ready_.push_back(process);
    return process;
}

std::string Scheduler::nextAutoNameLocked() {
    // p01, p02, ..., p99, p100, ... skipping names already taken by screen -s.
    while (true) {
        std::ostringstream name;
        name << 'p' << std::setw(2) << std::setfill('0') << nextAutoNumber_++;
        if (processes_.find(name.str()) == processes_.end()) {
            return name.str();
        }
    }
}

std::shared_ptr<Process> Scheduler::createProcess(const std::string& name) {
    std::vector<Instruction> program = randomProgram();  // outside mtx_ so the clock isn't held up
    std::lock_guard<std::mutex> lock(mtx_);
    if (processes_.find(name) != processes_.end()) {
        return std::shared_ptr<Process>();
    }
    return addProcessLocked(name, std::move(program));
}

std::shared_ptr<Process> Scheduler::findUnfinishedProcess(const std::string& name) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::map<std::string, std::shared_ptr<Process>>::iterator it = processes_.find(name);
    if (it == processes_.end() || it->second->isFinished()) {
        return std::shared_ptr<Process>();
    }
    return it->second;
}

bool Scheduler::startGenerating() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (generating_) {
        return false;
    }
    generating_ = true;
    ticksSinceBatch_ = 0;
    return true;
}

bool Scheduler::stopGenerating() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!generating_) {
        return false;
    }
    generating_ = false;
    return true;
}

std::string Scheduler::utilizationReport() {
    std::lock_guard<std::mutex> lock(mtx_);

    const int total = config_.numCpu;
    int used = 0;
    std::size_t nameWidth = 6;
    for (std::size_t i = 0; i < cores_.size(); ++i) {
        if (cores_[i]->process) {
            ++used;
            nameWidth = std::max(nameWidth, cores_[i]->process->name().size());
        }
    }
    for (std::size_t i = 0; i < finished_.size(); ++i) {
        nameWidth = std::max(nameWidth, finished_[i]->name().size());
    }
    const std::string rule(60, '-');

    std::ostringstream out;
    out << "CPU utilization: " << (used * 100 + total / 2) / total << "%\n"
        << "Cores used: " << used << '\n'
        << "Cores available: " << (total - used) << '\n'
        << "Ready queue: " << ready_.size() << " waiting, " << sleepers_.size() << " sleeping\n\n"
        << rule << '\n'
        << "Running processes:\n";
    if (used == 0) {
        out << "(none)\n";
    }
    for (std::size_t i = 0; i < cores_.size(); ++i) {
        const std::shared_ptr<Process>& process = cores_[i]->process;
        if (process) {
            out << std::left << std::setw(static_cast<int>(nameWidth)) << process->name() << "   ("
                << process->createdAt() << ")   Core: " << std::setw(3) << i << "   "
                << process->executedInstructions() << " / " << process->totalInstructions() << '\n';
        }
    }

    out << "\nFinished processes:\n";
    if (finished_.empty()) {
        out << "(none)\n";
    }
    for (std::size_t i = 0; i < finished_.size(); ++i) {
        const std::shared_ptr<Process>& process = finished_[i];
        out << std::left << std::setw(static_cast<int>(nameWidth)) << process->name() << "   ("
            << process->createdAt() << ")   Finished    " << process->totalInstructions() << " / "
            << process->totalInstructions() << '\n';
    }
    out << rule << '\n';
    return out.str();
}
