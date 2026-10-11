#include "apps/Calculator.h"

Calculator::Calculator() : App("Calculator", "calc.exe", ImVec2(80.0f, 60.0f), ImVec2(260.0f, 340.0f)) {}

void Calculator::renderContents() {
    // PLACEHOLDER (Phase 2): display, button grid, and arithmetic. See PLAN.md.
    ImGui::TextUnformatted("Calculator goes here.");
}
