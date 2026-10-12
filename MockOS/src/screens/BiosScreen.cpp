#include "screens/BiosScreen.h"

#include <algorithm>
#include <cstdio>
#include <utility>

#include "imgui.h"
#include "screens/ScreenUtil.h"
#include "ui/Fonts.h"
#include "ui/Theme.h"

namespace {

constexpr float FONT_SIZE = 20.0f;
constexpr float LOGO_FONT_SIZE = 30.0f;
constexpr ImVec2 LOGO_SIZE(170.0f, 70.0f);
constexpr float MARGIN = 24.0f;

constexpr double MEMORY_TEST_SECONDS = 1.6;
constexpr double DRIVE_INTERVAL_SECONDS = 0.35;
constexpr int DOT_COUNT = 12;
constexpr double DOTS_SECONDS = 0.8;
constexpr double HOLD_SECONDS = 0.8;  // pause on "Booting..." before the loading screen
constexpr int MAX_DRIVES_SHOWN = 6;

const ImVec4 TEXT_COLOR(0.75f, 0.75f, 0.75f, 1.0f);
const ImVec4 HEADER_COLOR(1.0f, 1.0f, 1.0f, 1.0f);

std::string orUnknown(const std::string& text) {
    return text.empty() ? "Unknown" : text;
}

}  // namespace

void BiosScreen::build() {
    info_ = querySystemInfo();

    double time = 0.0;
    auto add = [&](double delay, LineKind kind, std::string text) {
        time += delay;
        lines_.push_back({time, kind, std::move(text)});
    };
    char buffer[160];

    add(0.0, LineKind::Header, std::string(theme::OS_NAME) + " Modular BIOS v1.0, An Energy Star Ally");
    add(0.0, LineKind::Header, "Copyright (C) 2026, CSOPESY Group");
    add(0.0, LineKind::Text, "");

    add(0.4, LineKind::Text, "Main Processor : " + orUnknown(info_.cpuName));
    std::string speed = "CPU Speed      : ";
    speed += info_.cpuMHz > 0 ? std::to_string(info_.cpuMHz) + " MHz" : "Unknown";
    if (info_.cores > 0 && info_.threads > 0) {
        std::snprintf(buffer, sizeof(buffer), ", %d cores / %d threads", info_.cores, info_.threads);
        speed += buffer;
    }
    add(0.2, LineKind::Text, speed);
    add(0.3, LineKind::MemoryTest, "Memory Test    : ");
    time += MEMORY_TEST_SECONDS;  // the next lines wait for the count to finish

    add(0.2, LineKind::Text, "");
    add(0.0, LineKind::Text, "Mainboard      : " + orUnknown(info_.boardModel));
    if (!info_.boardMaker.empty()) {
        add(0.0, LineKind::Text, "Manufacturer   : " + info_.boardMaker);
    }
    add(0.2, LineKind::Text, "Display        : " + orUnknown(info_.gpuName));

    add(0.0, LineKind::Text, "");
    add(0.3, LineKind::Text, "Detecting drives ...");
    const int driveCount = std::min(static_cast<int>(info_.drives.size()), MAX_DRIVES_SHOWN);
    for (int i = 0; i < driveCount; ++i) {
        const DriveInfo& drive = info_.drives[i];
        if (drive.sizeGB > 0.0) {
            std::snprintf(buffer, sizeof(buffer), "  %-4s %d : %-34s %8.1f GB", drive.bus.c_str(), i,
                          drive.model.c_str(), drive.sizeGB);
        } else {
            std::snprintf(buffer, sizeof(buffer), "  %-4s %d : %s", drive.bus.c_str(), i, drive.model.c_str());
        }
        add(DRIVE_INTERVAL_SECONDS, LineKind::Text, buffer);
    }
    if (driveCount == 0) {
        add(DRIVE_INTERVAL_SECONDS, LineKind::Text, "  None found");
    }

    add(0.0, LineKind::Text, "");
    add(0.4, LineKind::Dots, "Verifying DMI Pool Data ");
    time += DOTS_SECONDS;
    add(0.2, LineKind::Text, std::string("Booting ") + theme::OS_NAME + "...");
    doneAt_ = time + HOLD_SECONDS;
    built_ = true;
}

void BiosScreen::render(double elapsed) {
    if (!built_) {
        build();
    }

    beginFullscreenWindow("##bios", ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    renderLogo();
    ImGui::PushFont(fonts::mono(), FONT_SIZE);
    ImGui::SetCursorPos(ImVec2(MARGIN, MARGIN));

    char buffer[64];
    for (const Line& line : lines_) {
        if (elapsed < line.showAt) {
            break;  // lines are in time order
        }
        const double since = elapsed - line.showAt;
        std::string text = line.text;
        if (line.kind == LineKind::MemoryTest) {
            // Count up in whole megabytes, like a real POST memory test.
            const double progress = std::min(1.0, since / MEMORY_TEST_SECONDS);
            const unsigned long long counted = static_cast<unsigned long long>(info_.memoryKB * progress) / 1024 * 1024;
            std::snprintf(buffer, sizeof(buffer), "%lluK%s", counted, progress >= 1.0 ? " OK" : "");
            text += buffer;
        } else if (line.kind == LineKind::Dots) {
            const int dots = std::min(DOT_COUNT, static_cast<int>(since / DOTS_SECONDS * DOT_COUNT));
            text.append(dots, '.');
        }
        ImGui::SetCursorPosX(MARGIN);
        ImGui::TextColored(line.kind == LineKind::Header ? HEADER_COLOR : TEXT_COLOR, "%s", text.c_str());
    }

    // Footer, pinned to the bottom like on a real POST screen.
    ImGui::SetCursorPos(ImVec2(MARGIN, ImGui::GetWindowHeight() - MARGIN - ImGui::GetTextLineHeight()));
    ImGui::TextColored(TEXT_COLOR, "Press DEL to enter SETUP, F12 for Boot Menu");

    ImGui::PopFont();
    ImGui::End();
}

void BiosScreen::renderLogo() {
    // A pastel logo where real BIOSes put the Energy Star logo, matching the desktop theme.
    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 min(windowPos.x + ImGui::GetWindowWidth() - MARGIN - LOGO_SIZE.x, windowPos.y + MARGIN);
    const ImVec2 max(min.x + LOGO_SIZE.x, min.y + LOGO_SIZE.y);
    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(min, max, IM_COL32(166, 233, 220, 255),
                                                        IM_COL32(218, 196, 240, 255), IM_COL32(250, 214, 172, 255),
                                                        IM_COL32(252, 234, 156, 255));

    ImGui::PushFont(fonts::semibold(), LOGO_FONT_SIZE);
    const ImVec2 textSize = ImGui::CalcTextSize(theme::OS_NAME);
    ImGui::GetWindowDrawList()->AddText(
        ImVec2(min.x + (LOGO_SIZE.x - textSize.x) * 0.5f, min.y + (LOGO_SIZE.y - textSize.y) * 0.5f), theme::TEXT,
        theme::OS_NAME);
    ImGui::PopFont();
}

bool BiosScreen::isDone(double elapsed) const {
    return built_ && elapsed >= doneAt_;
}
