#pragma once

#include <random>
#include <string>
#include <vector>

#include "apps/App.h"

class AppManager;

class TaskManager : public App {
public:
    explicit TaskManager(AppManager& apps);

protected:
    void renderContents() override;

private:
    struct Process {
        std::string name;
        int pid;
        App* app;  // nullptr for a mock system process
        float baseCpu;
        float baseMemoryMb;
        float cpu;
        float memoryMb;
    };

    void syncAppProcesses();
    void driftUsage();
    void sortProcesses();
    void renderFooter();
    const Process* findProcess(int pid) const;

    AppManager& apps_;
    std::vector<Process> processes_;
    std::mt19937 rng_;
    int nextPid_ = 5208;
    int selectedPid_ = -1;
    double lastDrift_ = 0.0;
    bool rowsAdded_ = false;
};
