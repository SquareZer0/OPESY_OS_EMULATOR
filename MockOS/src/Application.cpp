#include "Application.h"

#include <cstdio>

#include <GLFW/glfw3.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "ui/Fonts.h"
#include "ui/Theme.h"

namespace {

constexpr int INITIAL_WIDTH = 1280;
constexpr int INITIAL_HEIGHT = 720;
// Below this size the taskbar and widgets stop fitting.
constexpr int MIN_WIDTH = 800;
constexpr int MIN_HEIGHT = 600;
const char* const GLSL_VERSION = "#version 130";  // matches the OpenGL 3.0 context requested in init()

void glfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

}  // namespace

int Application::run() {
    if (!init()) {
        return 1;
    }

    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();

        // Don't spin the CPU while minimized.
        if (glfwGetWindowAttrib(window_, GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderCurrentScreen();

        ImGui::Render();
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
    }

    cleanup();
    return 0;
}

bool Application::init() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window_ = glfwCreateWindow(INITIAL_WIDTH, INITIAL_HEIGHT, theme::OS_NAME, nullptr, nullptr);
    if (window_ == nullptr) {
        glfwTerminate();
        return false;
    }
    glfwSetWindowSizeLimits(window_, MIN_WIDTH, MIN_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);  // vsync: cap the frame rate at the monitor's refresh rate

    // The X button and Alt+F4 must not close the app on their own; see onCloseRequested().
    glfwSetWindowUserPointer(window_, this);
    glfwSetWindowCloseCallback(window_, windowCloseCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;  // don't carry window positions over between runs
    fonts::load();
    theme::apply();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    switchTo(AppState::Bios);
    return true;
}

void Application::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window_);
    window_ = nullptr;
    glfwTerminate();
}

void Application::renderCurrentScreen() {
    const double elapsed = glfwGetTime() - stateStartTime_;

    switch (state_) {
    case AppState::Bios:
        bios_.render(elapsed);
        if (bios_.isDone(elapsed)) {
            switchTo(AppState::Splash);
        }
        break;
    case AppState::Splash:
        splash_.render(elapsed);
        if (splash_.isDone(elapsed)) {
            switchTo(AppState::Desktop);
        }
        break;
    case AppState::Desktop:
        desktop_.render();
        if (desktop_.shutdownConfirmed()) {
            switchTo(AppState::ShuttingDown);
        }
        break;
    case AppState::ShuttingDown:
        shutdown_.render(elapsed);
        // Ending the main loop (instead of calling exit()) lets run() shut
        // ImGui and GLFW down in order before returning.
        if (shutdown_.isDone(elapsed)) {
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
        }
        break;
    }
}

void Application::switchTo(AppState next) {
    state_ = next;
    stateStartTime_ = glfwGetTime();
}

void Application::windowCloseCallback(GLFWwindow* window) {
    // Cancel the close. The PWR flow decides when the app actually exits.
    glfwSetWindowShouldClose(window, GLFW_FALSE);
    static_cast<Application*>(glfwGetWindowUserPointer(window))->onCloseRequested();
}

void Application::onCloseRequested() {
    // Treat the X button and Alt+F4 like pressing PWR. During boot and
    // shutdown there is no dialog to show, so the request is ignored.
    if (state_ == AppState::Desktop) {
        desktop_.requestPowerDialog();
    }
}
