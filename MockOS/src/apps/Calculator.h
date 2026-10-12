#pragma once

#include <string>

#include "apps/App.h"

class Calculator : public App {
public:
    Calculator();

protected:
    void renderContents() override;

private:
    void handleKeyboard();
    void press(char key);
    void pressDigit(char key);
    void pressOperator(char op);
    void pressEquals();
    void pressPercent();
    void toggleSign();
    void backspace();
    void clearAll();
    void showError();
    double entryValue() const;

    std::string entry_ = "0";  // the big number on the display
    std::string history_;      // the small line above it, e.g. "12 +"
    double stored_ = 0.0;      // left-hand side of the pending operation
    char pendingOp_ = 0;       // '+', '-', '*', '/' or 0 for none
    bool typing_ = false;      // the next digit is appended to entry_ instead of replacing it
    bool hasEntry_ = false;    // a number was entered after the last operator
    bool error_ = false;
};
