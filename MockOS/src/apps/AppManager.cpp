#include "apps/AppManager.h"

#include "apps/Calculator.h"
#include "apps/Notepad.h"
#include "apps/TaskManager.h"

AppManager::AppManager() {
    // Taskbar buttons appear in this order.
    apps_.push_back(std::make_unique<Calculator>());
    apps_.push_back(std::make_unique<Notepad>());
    apps_.push_back(std::make_unique<TaskManager>(*this));
}

void AppManager::render() {
    for (const auto& app : apps_) {
        if (app->render()) {
            frontApp_ = app.get();
        }
    }
    if (frontApp_ != nullptr && !frontApp_->isOpen()) {
        frontApp_ = nullptr;
    }
}

void AppManager::onTaskbarClick(App& app) {
    // Clicking the taskbar moves focus to the taskbar itself, so frontApp_
    // (not the current focus) tells which app was in front before the click.
    if (app.isOpen() && &app == frontApp_) {
        app.close();
    } else {
        app.open();
    }
}
