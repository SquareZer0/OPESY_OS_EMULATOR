#include "apps/TaskManager.h"

#include "apps/AppManager.h"

TaskManager::TaskManager(const AppManager& apps)
    : App("Task Manager", "taskmgr.exe", ImVec2(200.0f, 140.0f), ImVec2(560.0f, 380.0f)), apps_(apps) {}

void TaskManager::renderContents() {
    // PLACEHOLDER (Phase 2): process table with mock CPU and memory values,
    // mixing fake system processes with the open apps listed here. See PLAN.md.
    ImGui::TextUnformatted("Open apps:");
    for (const auto& app : apps_.list()) {
        if (app->isOpen()) {
            ImGui::BulletText("%s", app->processName().c_str());
        }
    }
}
