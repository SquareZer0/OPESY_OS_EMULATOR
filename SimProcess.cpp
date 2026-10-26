#include "SimProcess.h"

#include <sstream>

#include "Console.h"

namespace {

// Variables are uint16, so results are clamped to 0-65535 instead of wrapping.
std::uint16_t clampToUint16(std::int64_t value) {
    if (value < 0) {
        return 0;
    }
    if (value > 65535) {
        return 65535;
    }
    return static_cast<std::uint16_t>(value);
}

}  // namespace

Process::Process(int id, const std::string& name, std::vector<Instruction> program)
    : id_(id),
      name_(name),
      createdAt_(console::timestamp()),
      program_(std::move(program)),
      total_(countInstructions(program_)) {
    Frame whole = {&program_, 0, 1};
    stack_.push_back(whole);
}

StepResult Process::executeNext(int coreId) {
    std::lock_guard<std::mutex> lock(mtx_);
    StepResult result = {StepResult::Continue, 0};
    while (executed_ < total_ && !stack_.empty()) {
        Frame& frame = stack_.back();

        // End of a list: repeat it if it's a FOR body with repeats left,
        // otherwise go back to the enclosing list and move past the FOR.
        if (frame.pc >= frame.list->size()) {
            if (--frame.iterationsLeft > 0) {
                frame.pc = 0;
            } else {
                stack_.pop_back();
                if (!stack_.empty()) {
                    ++stack_.back().pc;
                }
            }
            continue;
        }

        const Instruction& instruction = (*frame.list)[frame.pc];
        if (instruction.op == OpCode::For) {
            if (instruction.repeats == 0 || !instruction.body || instruction.body->empty()) {
                ++frame.pc;
            } else {
                Frame body = {instruction.body.get(), 0, instruction.repeats};
                stack_.push_back(body);  // `frame` is not used after this
            }
            continue;
        }

        run(instruction, coreId, result);
        ++frame.pc;
        ++executed_;
        break;
    }
    if (executed_ >= total_) {
        result.kind = StepResult::Finished;
    }
    return result;
}

std::uint16_t Process::read(const Operand& operand) {
    if (!operand.isVariable) {
        return operand.value;
    }
    // A variable that was never declared is created with the value 0.
    return variables_[VARIABLE_NAMES[operand.variable]];
}

void Process::run(const Instruction& instruction, int coreId, StepResult& result) {
    switch (instruction.op) {
    case OpCode::Print: {
        std::ostringstream message;
        if (instruction.a.isVariable) {
            message << "Value of " << VARIABLE_NAMES[instruction.a.variable] << ": " << read(instruction.a);
        } else {
            message << "Hello world from " << name_ << "!";
        }
        logs_.push_back("(" + console::timestamp() + ") Core:" + std::to_string(coreId) + " \"" + message.str() +
                        "\"");
        break;
    }
    case OpCode::Declare:
        variables_[VARIABLE_NAMES[instruction.target]] = instruction.a.value;
        break;
    case OpCode::Add:
        variables_[VARIABLE_NAMES[instruction.target]] =
            clampToUint16(static_cast<std::int64_t>(read(instruction.a)) + read(instruction.b));
        break;
    case OpCode::Subtract:
        variables_[VARIABLE_NAMES[instruction.target]] =
            clampToUint16(static_cast<std::int64_t>(read(instruction.a)) - read(instruction.b));
        break;
    case OpCode::Sleep:
        if (instruction.sleepTicks > 0) {
            result.kind = StepResult::Sleep;
            result.sleepTicks = instruction.sleepTicks;
        }
        break;
    case OpCode::For:
        break;  // handled by executeNext
    }
}

std::uint64_t Process::executedInstructions() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return executed_;
}

bool Process::isFinished() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return executed_ >= total_;
}

std::string Process::smiReport() const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::ostringstream out;
    out << "Process name: " << name_ << '\n' << "ID: " << id_ << '\n' << "Logs:\n";
    if (logs_.empty()) {
        out << "(no logs yet)\n";
    }
    for (std::size_t i = 0; i < logs_.size(); ++i) {
        out << logs_[i] << '\n';
    }
    out << '\n';
    if (executed_ >= total_) {
        out << "Finished!\n";
    } else {
        out << "Current instruction line: " << executed_ << '\n' << "Lines of code: " << total_ << '\n';
    }
    return out.str();
}
