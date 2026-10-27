#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

enum class OpCode : std::uint8_t { Print, Declare, Add, Subtract, Sleep, For };

// Generated programs use this small pool of variable names.
const int VARIABLE_COUNT = 5;
extern const char* const VARIABLE_NAMES[VARIABLE_COUNT];

// FOR loops can be nested up to this many levels deep.
const int MAX_FOR_DEPTH = 3;

// A value an instruction reads: either a variable or a literal number.
struct Operand {
    bool isVariable = false;
    std::uint8_t variable = 0;  // index into VARIABLE_NAMES
    std::uint16_t value = 0;    // used when isVariable is false
};

// One instruction of a simulated process:
//   PRINT(msg)            message is "Hello world from <name>!" (generated programs always use this);
//                         a.isVariable instead prints a variable's value, for a test case that asks for it
//   DECLARE(var, value)   target = a.value
//   ADD(var, a, b)        target = a + b, clamped to 0-65535
//   SUBTRACT(var, a, b)   target = a - b, clamped to 0-65535
//   SLEEP(X)              the process gives up its core for sleepTicks ticks
//   FOR([body], repeats)  runs body `repeats` times
struct Instruction {
    OpCode op = OpCode::Print;
    std::uint8_t target = 0;  // variable written by DECLARE/ADD/SUBTRACT
    Operand a;
    Operand b;
    std::uint8_t sleepTicks = 0;
    std::uint64_t repeats = 0;
    std::shared_ptr<std::vector<Instruction>> body;
};

// Builds a random program that executes exactly `total` instructions, with FOR
// bodies counted once per repeat. FOR itself doesn't count; it only repeats
// what's inside it.
std::vector<Instruction> generateProgram(std::uint64_t total, std::mt19937& rng);

// How many instructions `program` executes, counting FOR bodies once per repeat.
std::uint64_t countInstructions(const std::vector<Instruction>& program);

// Readable form of one instruction, e.g. "ADD(x, y, 5)" or "FOR([3 instructions], 4)".
std::string describe(const Instruction& instruction);

#endif
