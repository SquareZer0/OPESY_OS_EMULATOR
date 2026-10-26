#include "Instruction.h"

#include <algorithm>
#include <sstream>

const char* const VARIABLE_NAMES[VARIABLE_COUNT] = {"x", "y", "z", "a", "b"};

namespace {

// Generation tuning. Lists are kept short so huge instruction counts (min-ins
// and max-ins can go up to 2^32) are reached with FOR loops instead of
// billions of stored instructions.
const std::size_t MAX_LIST_LENGTH = 50;  // past this, the rest of a list is wrapped in a loop
const std::uint64_t MAX_BODY_INSTRUCTIONS = 8;
const std::uint64_t MAX_RANDOM_REPEATS = 5;
const int FOR_CHANCE_PERCENT = 10;
const int MAX_SLEEP_TICKS = 10;
const int MAX_RANDOM_VALUE = 500;

std::uint64_t randomBetween(std::mt19937& rng, std::uint64_t low, std::uint64_t high) {
    std::uniform_int_distribution<std::uint64_t> distribution(low, high);
    return distribution(rng);
}

std::uint8_t randomVariable(std::mt19937& rng) {
    return static_cast<std::uint8_t>(randomBetween(rng, 0, VARIABLE_COUNT - 1));
}

Operand randomOperand(std::mt19937& rng) {
    Operand operand;
    operand.isVariable = randomBetween(rng, 0, 1) == 0;
    if (operand.isVariable) {
        operand.variable = randomVariable(rng);
    } else {
        operand.value = static_cast<std::uint16_t>(randomBetween(rng, 0, MAX_RANDOM_VALUE));
    }
    return operand;
}

Instruction randomLeaf(std::mt19937& rng) {
    Instruction instruction;
    const std::uint64_t roll = randomBetween(rng, 0, 99);
    if (roll < 25) {
        instruction.op = OpCode::Print;
        instruction.a.isVariable = randomBetween(rng, 0, 1) == 0;
        instruction.a.variable = randomVariable(rng);
    } else if (roll < 45) {
        instruction.op = OpCode::Declare;
        instruction.target = randomVariable(rng);
        instruction.a.value = static_cast<std::uint16_t>(randomBetween(rng, 0, MAX_RANDOM_VALUE));
    } else if (roll < 67) {
        instruction.op = OpCode::Add;
        instruction.target = randomVariable(rng);
        instruction.a = randomOperand(rng);
        instruction.b = randomOperand(rng);
    } else if (roll < 89) {
        instruction.op = OpCode::Subtract;
        instruction.target = randomVariable(rng);
        instruction.a = randomOperand(rng);
        instruction.b = randomOperand(rng);
    } else {
        instruction.op = OpCode::Sleep;
        instruction.sleepTicks = static_cast<std::uint8_t>(randomBetween(rng, 1, MAX_SLEEP_TICKS));
    }
    return instruction;
}

// Builds a list that executes exactly `total` instructions. `depth` is how
// many FOR loops already enclose it.
std::vector<Instruction> generateList(std::uint64_t total, int depth, std::mt19937& rng) {
    std::vector<Instruction> list;
    std::uint64_t count = 0;
    while (count < total) {
        const std::uint64_t remaining = total - count;
        const bool canLoop = depth < MAX_FOR_DEPTH && remaining >= 2;
        const bool mustLoop = canLoop && list.size() >= MAX_LIST_LENGTH;
        if (canLoop && (mustLoop || randomBetween(rng, 0, 99) < FOR_CHANCE_PERCENT)) {
            const std::uint64_t bodyCount = randomBetween(rng, 1, std::min(remaining / 2, MAX_BODY_INSTRUCTIONS));
            const std::uint64_t maxRepeats = remaining / bodyCount;  // at least 2
            const std::uint64_t repeats =
                mustLoop ? maxRepeats : randomBetween(rng, 2, std::min(maxRepeats, MAX_RANDOM_REPEATS));
            Instruction loop;
            loop.op = OpCode::For;
            loop.repeats = repeats;
            loop.body = std::make_shared<std::vector<Instruction>>(generateList(bodyCount, depth + 1, rng));
            list.push_back(loop);
            count += bodyCount * repeats;
        } else {
            list.push_back(randomLeaf(rng));
            ++count;
        }
    }
    return list;
}

std::string describeOperand(const Operand& operand) {
    if (operand.isVariable) {
        return VARIABLE_NAMES[operand.variable];
    }
    std::ostringstream text;
    text << operand.value;
    return text.str();
}

}  // namespace

std::vector<Instruction> generateProgram(std::uint64_t total, std::mt19937& rng) {
    return generateList(total, 0, rng);
}

std::uint64_t countInstructions(const std::vector<Instruction>& program) {
    std::uint64_t count = 0;
    for (std::size_t i = 0; i < program.size(); ++i) {
        const Instruction& instruction = program[i];
        if (instruction.op == OpCode::For) {
            count += instruction.body ? instruction.repeats * countInstructions(*instruction.body) : 0;
        } else {
            ++count;
        }
    }
    return count;
}

std::string describe(const Instruction& instruction) {
    std::ostringstream text;
    switch (instruction.op) {
    case OpCode::Print:
        if (instruction.a.isVariable) {
            text << "PRINT(\"Value of " << VARIABLE_NAMES[instruction.a.variable] << ": \" + "
                 << VARIABLE_NAMES[instruction.a.variable] << ")";
        } else {
            text << "PRINT(\"Hello world from <name>!\")";
        }
        break;
    case OpCode::Declare:
        text << "DECLARE(" << VARIABLE_NAMES[instruction.target] << ", " << instruction.a.value << ")";
        break;
    case OpCode::Add:
        text << "ADD(" << VARIABLE_NAMES[instruction.target] << ", " << describeOperand(instruction.a) << ", "
             << describeOperand(instruction.b) << ")";
        break;
    case OpCode::Subtract:
        text << "SUBTRACT(" << VARIABLE_NAMES[instruction.target] << ", " << describeOperand(instruction.a) << ", "
             << describeOperand(instruction.b) << ")";
        break;
    case OpCode::Sleep:
        text << "SLEEP(" << static_cast<int>(instruction.sleepTicks) << ")";
        break;
    case OpCode::For:
        text << "FOR([" << (instruction.body ? instruction.body->size() : 0) << " instructions], "
             << instruction.repeats << ")";
        break;
    }
    return text.str();
}
