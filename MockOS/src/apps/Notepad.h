#pragma once

#include <string>

#include "apps/App.h"

class Notepad : public App {
public:
    Notepad();

protected:
    void renderContents() override;
    ImGuiWindowFlags windowFlags() const override { return ImGuiWindowFlags_MenuBar; }

private:
    void renderMenuBar();
    void renderStatusLine();

    std::string text_;
    double savedAt_ = -1.0;  // ImGui::GetTime() of the last Save, for the "Saved" note
    bool focusText_ = false;
};
