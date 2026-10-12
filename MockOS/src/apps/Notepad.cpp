#include "apps/Notepad.h"

#include <cctype>
#include <cfloat>

#include "misc/cpp/imgui_stdlib.h"
#include "ui/Theme.h"

namespace {

constexpr double SAVED_NOTE_SECONDS = 2.0;

int countWords(const std::string& text) {
    int words = 0;
    bool inWord = false;
    for (unsigned char c : text) {
        const bool space = std::isspace(c) != 0;
        if (!space && !inWord) {
            ++words;
        }
        inWord = !space;
    }
    return words;
}

// Counts characters, not bytes (UTF-8 continuation bytes are skipped), and leaves out line breaks.
int countCharacters(const std::string& text) {
    int count = 0;
    for (unsigned char c : text) {
        if ((c & 0xC0) != 0x80 && c != '\n') {
            ++count;
        }
    }
    return count;
}

}  // namespace

Notepad::Notepad() : App("Notepad", "notepad.exe", icons::notepad, ImVec2(140.0f, 100.0f), ImVec2(480.0f, 360.0f)) {}

void Notepad::renderContents() {
    // Nothing is really saved, so a reopened Notepad starts empty.
    if (ImGui::IsWindowAppearing()) {
        text_.clear();
        savedAt_ = -1.0;
        focusText_ = true;
    }

    renderMenuBar();

    if (focusText_) {
        ImGui::SetKeyboardFocusHere();
        focusText_ = false;
    }
    const float statusHeight = ImGui::GetTextLineHeightWithSpacing();
    ImGui::InputTextMultiline("##text", &text_, ImVec2(-FLT_MIN, -statusHeight), ImGuiInputTextFlags_WordWrap);

    renderStatusLine();
}

void Notepad::renderMenuBar() {
    if (!ImGui::BeginMenuBar()) {
        return;
    }
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New")) {
            text_.clear();
            savedAt_ = -1.0;
            focusText_ = true;
        }
        if (ImGui::MenuItem("Save")) {
            savedAt_ = ImGui::GetTime();  // mock: nothing is written to disk
        }
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void Notepad::renderStatusLine() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(theme::TEXT_MUTED));
    ImGui::Text("Words: %d   Characters: %d", countWords(text_), countCharacters(text_));
    ImGui::PopStyleColor();

    if (savedAt_ >= 0.0 && ImGui::GetTime() - savedAt_ < SAVED_NOTE_SECONDS) {
        const char* note = "Saved";
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(note).x);
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(theme::ACCENT));
        ImGui::TextUnformatted(note);
        ImGui::PopStyleColor();
    }
}
