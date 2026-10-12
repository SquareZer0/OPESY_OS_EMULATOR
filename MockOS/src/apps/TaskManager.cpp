#include "apps/TaskManager.h"

#include <algorithm>
#include <cctype>

#include "apps/AppManager.h"
#include "ui/Theme.h"

namespace {

enum Column { NAME, PID, STATUS, CPU, MEMORY };

constexpr double DRIFT_SECONDS = 1.0;

constexpr float SIDEBAR_WIDTH = 170.0f;

struct SidebarItem {
    const char* label;
    icons::LineIconFn icon;
};

const SidebarItem SIDEBAR[] = {
    {"Processes", icons::processes},      {"Performance", icons::performance}, {"App History", icons::appHistory},
    {"Startup Apps", icons::startupApps}, {"Details", icons::details},         {"Services", icons::services},
};

bool lessIgnoreCase(const std::string& a, const std::string& b) {
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](unsigned char x, unsigned char y) {
        return std::tolower(x) < std::tolower(y);
    });
}

}  // namespace

TaskManager::TaskManager(AppManager& apps)
    : App("Task Manager", "taskmgr.exe", icons::taskManager, ImVec2(200.0f, 140.0f), ImVec2(720.0f, 420.0f),
          ImVec2(700.0f, 380.0f)),
      apps_(apps),
      rng_(std::random_device{}()) {
    struct Mock {
        const char* name;
        int pid;
        float cpu;
        float memoryMb;
    };
    const Mock systemProcesses[] = {
        {"System", 4, 0.4f, 0.2f},          {"Registry", 120, 0.0f, 48.5f},
        {"smss.exe", 412, 0.0f, 1.1f},      {"csrss.exe", 620, 0.2f, 5.6f},
        {"wininit.exe", 704, 0.0f, 6.8f},   {"services.exe", 812, 0.1f, 9.4f},
        {"lsass.exe", 836, 0.1f, 14.2f},    {"svchost.exe", 1024, 0.3f, 22.7f},
        {"svchost.exe", 1168, 0.2f, 12.9f}, {"dwm.exe", 1340, 1.8f, 96.3f},
        {"explorer.exe", 4128, 0.9f, 118.4f},
    };
    for (const Mock& m : systemProcesses) {
        processes_.push_back({m.name, m.pid, nullptr, m.cpu, m.memoryMb, m.cpu, m.memoryMb});
    }
}

// Adds a row for every newly opened app and drops rows of closed ones. A
// reopened app gets a new PID, like a real process.
void TaskManager::syncAppProcesses() {
    processes_.erase(std::remove_if(processes_.begin(), processes_.end(),
                                    [](const Process& p) { return p.app != nullptr && !p.app->isOpen(); }),
                     processes_.end());

    std::uniform_real_distribution<float> cpu(0.5f, 3.0f);
    std::uniform_real_distribution<float> memory(15.0f, 60.0f);
    for (const auto& app : apps_.list()) {
        const bool listed = std::any_of(processes_.begin(), processes_.end(),
                                        [&](const Process& p) { return p.app == app.get(); });
        if (app->isOpen() && !listed) {
            const float c = cpu(rng_);
            const float m = memory(rng_);
            processes_.push_back({app->processName(), nextPid_, app.get(), c, m, c, m});
            nextPid_ += 4;  // Windows PIDs are multiples of 4
            rowsAdded_ = true;
        }
    }

    if (findProcess(selectedPid_) == nullptr) {
        selectedPid_ = -1;
    }
}

// Nudges every CPU and memory value once a second, staying near each
// process's starting value.
void TaskManager::driftUsage() {
    const double now = ImGui::GetTime();
    if (now - lastDrift_ < DRIFT_SECONDS) {
        return;
    }
    lastDrift_ = now;

    std::uniform_real_distribution<float> step(-1.0f, 1.0f);
    for (Process& p : processes_) {
        p.cpu = std::clamp(p.cpu + step(rng_) * 0.5f, 0.0f, p.baseCpu * 2.0f + 1.0f);
        p.memoryMb = std::clamp(p.memoryMb + step(rng_) * p.baseMemoryMb * 0.03f, p.baseMemoryMb * 0.85f,
                                p.baseMemoryMb * 1.15f);
    }
}

// Only re-sorts when a header is clicked or a row is added, so rows don't move
// around while their values drift.
void TaskManager::sortProcesses() {
    ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs();
    if (specs == nullptr || specs->SpecsCount == 0 || (!specs->SpecsDirty && !rowsAdded_)) {
        return;
    }
    specs->SpecsDirty = false;
    rowsAdded_ = false;
    const ImGuiID column = specs->Specs[0].ColumnUserID;
    const bool ascending = specs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;

    auto less = [column](const Process& a, const Process& b) {
        switch (column) {
            case NAME:
                return lessIgnoreCase(a.name, b.name);
            case PID:
                return a.pid < b.pid;
            case CPU:
                return a.cpu < b.cpu;
            default:
                return a.memoryMb < b.memoryMb;
        }
    };
    std::stable_sort(processes_.begin(), processes_.end(),
                     [&](const Process& a, const Process& b) { return ascending ? less(a, b) : less(b, a); });
}

void TaskManager::renderContents() {
    syncAppProcesses();
    driftUsage();

    const float height = ImGui::GetContentRegionAvail().y;
    renderSidebar(height);
    ImGui::SameLine();
    theme::beginGlassCard("##processesCard", ImVec2(ImGui::GetContentRegionAvail().x, height));
    renderTable();
    renderFooter();
    ImGui::EndChild();
}

// Only Processes is built; the other pages are greyed out and can't be opened.
void TaskManager::renderSidebar(float height) {
    theme::beginGlassCard("##sidebar", ImVec2(SIDEBAR_WIDTH, height));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 2.0f));
    for (int i = 0; i < static_cast<int>(sizeof(SIDEBAR) / sizeof(SIDEBAR[0])); ++i) {
        theme::sidebarItem(SIDEBAR[i].label, SIDEBAR[i].icon, i == 0, i == 0);
    }

    ImGui::PopStyleVar();
    ImGui::EndChild();
}

void TaskManager::renderTable() {
    const ImGuiTableFlags flags = ImGuiTableFlags_Sortable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp;
    const float footerHeight = ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginTable("processes", 5, flags, ImVec2(0.0f, -footerHeight))) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort, 2.5f, NAME);
        ImGui::TableSetupColumn("PID", 0, 1.0f, PID);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_NoSort, 1.5f, STATUS);
        ImGui::TableSetupColumn("CPU %", ImGuiTableColumnFlags_PreferSortDescending, 1.0f, CPU);
        ImGui::TableSetupColumn("Memory (MB)", ImGuiTableColumnFlags_PreferSortDescending, 2.0f, MEMORY);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        sortProcesses();

        for (const Process& p : processes_) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(p.pid);
            if (ImGui::Selectable(p.name.c_str(), p.pid == selectedPid_, ImGuiSelectableFlags_SpanAllColumns)) {
                selectedPid_ = p.pid;
            }
            ImGui::PopID();
            ImGui::TableNextColumn();
            ImGui::Text("%d", p.pid);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Running");
            ImGui::TableNextColumn();
            ImGui::Text("%.1f", p.cpu);
            ImGui::TableNextColumn();
            ImGui::Text("%.1f", p.memoryMb);
        }
        ImGui::EndTable();
    }
}

// Process count on the left, End task on the right. Only app rows can be
// ended; system processes leave the button disabled.
void TaskManager::renderFooter() {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Processes: %d", static_cast<int>(processes_.size()));

    const Process* selected = findProcess(selectedPid_);
    const bool canEnd = selected != nullptr && selected->app != nullptr;
    const char* label = "End task";
    const float width = ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width);

    ImGui::BeginDisabled(!canEnd);
    if (theme::accentButton(label) && canEnd) {
        selected->app->close();
        selectedPid_ = -1;
    }
    ImGui::EndDisabled();
}

const TaskManager::Process* TaskManager::findProcess(int pid) const {
    for (const Process& p : processes_) {
        if (p.pid == pid) {
            return &p;
        }
    }
    return nullptr;
}
