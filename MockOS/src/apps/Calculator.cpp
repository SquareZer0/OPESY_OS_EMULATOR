#include "apps/Calculator.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "ui/Fonts.h"
#include "ui/Theme.h"

namespace {

// UTF-8 escapes so the source compiles the same with or without /utf-8.
const char* const DIVIDE = "\xC3\xB7";
const char* const TIMES = "\xC3\x97";
const char* const MINUS = "\xE2\x88\x92";
const char* const PLUS_MINUS = "\xC2\xB1";

constexpr float DISPLAY_FONT_SIZE = 30.0f;
constexpr std::size_t MAX_ENTRY_LENGTH = 16;

struct Key {
    const char* label;
    char action;  // 'E' = CE, 'C' = C, 'n' = plus/minus, 'B' = backspace (keyboard only)
};

const Key KEYS[5][4] = {
    {{"%", '%'}, {"CE", 'E'}, {"C", 'C'}, {DIVIDE, '/'}},
    {{"7", '7'}, {"8", '8'}, {"9", '9'}, {TIMES, '*'}},
    {{"4", '4'}, {"5", '5'}, {"6", '6'}, {MINUS, '-'}},
    {{"1", '1'}, {"2", '2'}, {"3", '3'}, {"+", '+'}},
    {{PLUS_MINUS, 'n'}, {"0", '0'}, {".", '.'}, {"=", '='}},
};

const char* symbolFor(char op) {
    switch (op) {
        case '/':
            return DIVIDE;
        case '*':
            return TIMES;
        case '-':
            return MINUS;
        default:
            return "+";
    }
}

std::string format(double value) {
    if (value == 0.0) {
        return "0";  // avoids "-0"
    }
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.12g", value);
    return buffer;
}

// False on divide by zero or a result too big to show.
bool apply(double lhs, char op, double rhs, double& result) {
    switch (op) {
        case '+':
            result = lhs + rhs;
            break;
        case '-':
            result = lhs - rhs;
            break;
        case '*':
            result = lhs * rhs;
            break;
        default:
            if (rhs == 0.0) {
                return false;
            }
            result = lhs / rhs;
            break;
    }
    return std::isfinite(result);
}

void textRightAligned(const std::string& text) {
    const float width = ImGui::CalcTextSize(text.c_str()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, ImGui::GetContentRegionAvail().x - width));
    ImGui::TextUnformatted(text.c_str());
}

}  // namespace

Calculator::Calculator()
    : App("Calculator", "calc.exe", icons::calculator, ImVec2(80.0f, 60.0f), ImVec2(260.0f, 340.0f)) {}

void Calculator::renderContents() {
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        handleKeyboard();
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(theme::TEXT_MUTED));
    textRightAligned(history_.empty() ? " " : history_);
    ImGui::PopStyleColor();

    // Shrink the display font when a long number wouldn't fit.
    const float avail = ImGui::GetContentRegionAvail().x;
    ImGui::PushFont(fonts::semibold(), DISPLAY_FONT_SIZE);
    const float width = ImGui::CalcTextSize(entry_.c_str()).x;
    ImGui::PopFont();
    const float size = width > avail ? DISPLAY_FONT_SIZE * avail / width : DISPLAY_FONT_SIZE;
    ImGui::PushFont(fonts::semibold(), size);
    textRightAligned(entry_);
    ImGui::PopFont();
    ImGui::Spacing();

    // The grid stretches to fill the rest of the window.
    const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
    const ImVec2 region = ImGui::GetContentRegionAvail();
    const ImVec2 buttonSize((region.x - spacing.x * 3.0f) / 4.0f,
                            std::max(ImGui::GetFrameHeight(), (region.y - spacing.y * 4.0f) / 5.0f));
    for (const auto& row : KEYS) {
        for (int c = 0; c < 4; ++c) {
            if (c > 0) {
                ImGui::SameLine();
            }
            const Key& key = row[c];
            const bool pressed = key.action == '=' ? theme::accentButton(key.label, buttonSize)
                                                   : ImGui::Button(key.label, buttonSize);
            if (pressed) {
                press(key.action);
            }
        }
    }
}

// Uses the typed characters rather than key codes, so Shift+8 gives '*' and
// the number pad works on any keyboard layout.
void Calculator::handleKeyboard() {
    for (const ImWchar c : ImGui::GetIO().InputQueueCharacters) {
        if ((c >= '0' && c <= '9') || c == '.' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
            c == '=') {
            press(static_cast<char>(c));
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false)) {
        press('=');
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
        press('C');
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
        press('E');
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
        press('B');
    }
}

void Calculator::press(char key) {
    if (key == 'C') {
        clearAll();
        return;
    }
    if (error_) {
        // After an error only CE, backspace, a digit or the decimal point start over.
        if (key != 'E' && key != 'B' && key != '.' && !(key >= '0' && key <= '9')) {
            return;
        }
        clearAll();
    }

    switch (key) {
        case 'E':
            entry_ = "0";
            typing_ = false;
            hasEntry_ = true;
            break;
        case '+':
        case '-':
        case '*':
        case '/':
            pressOperator(key);
            break;
        case '=':
            pressEquals();
            break;
        case '%':
            pressPercent();
            break;
        case 'n':
            toggleSign();
            break;
        case 'B':
            backspace();
            break;
        default:
            pressDigit(key);
            break;
    }
}

void Calculator::pressDigit(char key) {
    if (!typing_) {
        if (pendingOp_ == 0) {
            history_.clear();  // starting a new calculation after "="
        }
        entry_ = "0";
        typing_ = true;
        hasEntry_ = true;
    }
    if (key == '.') {
        if (entry_.find('.') == std::string::npos) {
            entry_ += '.';
        }
    } else if (entry_ == "0") {
        entry_ = key;
    } else if (entry_.size() < MAX_ENTRY_LENGTH) {
        entry_ += key;
    }
}

// Runs the pending operation first, so 5 + 3 x 2 = gives 16, like a basic calculator.
void Calculator::pressOperator(char op) {
    if (pendingOp_ != 0 && hasEntry_) {
        double result = 0.0;
        if (!apply(stored_, pendingOp_, entryValue(), result)) {
            showError();
            return;
        }
        stored_ = result;
        entry_ = format(result);
    } else if (pendingOp_ == 0) {
        stored_ = entryValue();
    }
    pendingOp_ = op;
    typing_ = false;
    hasEntry_ = false;
    history_ = format(stored_) + " " + symbolFor(op);
}

void Calculator::pressEquals() {
    if (pendingOp_ == 0) {
        return;
    }
    // "5 + =" uses 5 again, giving 10.
    const double rhs = hasEntry_ ? entryValue() : stored_;
    double result = 0.0;
    if (!apply(stored_, pendingOp_, rhs, result)) {
        showError();
        return;
    }
    history_ = format(stored_) + " " + symbolFor(pendingOp_) + " " + format(rhs) + " =";
    entry_ = format(result);
    pendingOp_ = 0;
    typing_ = false;
    hasEntry_ = false;
}

// After + or -, % means a percentage of the left-hand side (50 + 10% = 55);
// otherwise it divides by 100.
void Calculator::pressPercent() {
    const double value = entryValue();
    const double percent = (pendingOp_ == '+' || pendingOp_ == '-') ? stored_ * value / 100.0 : value / 100.0;
    entry_ = format(percent);
    typing_ = false;
    hasEntry_ = true;
}

void Calculator::toggleSign() {
    if (entry_ == "0") {
        return;
    }
    entry_ = entry_[0] == '-' ? entry_.substr(1) : "-" + entry_;
    hasEntry_ = true;
}

// Only edits a number being typed; results stay as they are, like Windows.
void Calculator::backspace() {
    if (!typing_) {
        return;
    }
    entry_.pop_back();
    if (entry_.empty() || entry_ == "-") {
        entry_ = "0";
    }
}

void Calculator::clearAll() {
    entry_ = "0";
    history_.clear();
    stored_ = 0.0;
    pendingOp_ = 0;
    typing_ = false;
    hasEntry_ = false;
    error_ = false;
}

void Calculator::showError() {
    clearAll();
    entry_ = "Error";
    error_ = true;
}

double Calculator::entryValue() const {
    return std::strtod(entry_.c_str(), nullptr);
}
