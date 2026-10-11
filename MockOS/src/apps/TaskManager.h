#pragma once

#include "apps/App.h"

class AppManager;

class TaskManager : public App {
public:
    explicit TaskManager(const AppManager& apps);

protected:
    void renderContents() override;

private:
    const AppManager& apps_;  // to list the open apps as processes
};
