#pragma once

#include "screens/BiosScreen.h"
#include "screens/Desktop.h"
#include "screens/LoadingScreen.h"
#include "screens/ShutdownScreen.h"

struct GLFWwindow;

// The boot-to-shutdown flow, in order. The app only ever moves forward.
enum class AppState { Bios, Loading, Desktop, ShuttingDown };

// Owns the GLFW window and the Dear ImGui context, runs the main loop, and
// switches between screens. The only way out of run() is the PWR button.
class Application {
public:
    Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Returns the process exit code.
    int run();

private:
    bool init();
    void cleanup();
    void renderCurrentScreen();
    void switchTo(AppState next);
    void onCloseRequested();

    static void windowCloseCallback(GLFWwindow* window);

    GLFWwindow* window_ = nullptr;
    AppState state_ = AppState::Bios;
    double stateStartTime_ = 0.0;  // glfwGetTime() when state_ was entered

    BiosScreen bios_;
    LoadingScreen loading_;
    Desktop desktop_;
    ShutdownScreen shutdown_;
};
