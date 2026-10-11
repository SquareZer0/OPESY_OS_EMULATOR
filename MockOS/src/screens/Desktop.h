#pragma once

#include "apps/AppManager.h"
#include "screens/Taskbar.h"
#include "screens/TopBar.h"

// Wallpaper, app windows, top bar, taskbar dock, and the PWR confirmation dialog.
class Desktop {
public:
    void render();

    // Opens the "shut down?" dialog on the next frame. Used by the power
    // button and by the window's X button.
    void requestPowerDialog() { powerDialogRequested_ = true; }

    // True once the user has confirmed the shutdown.
    bool shutdownConfirmed() const { return shutdownConfirmed_; }

private:
    void renderPowerDialog();

    AppManager apps_;
    TopBar topBar_;
    Taskbar taskbar_;
    bool powerDialogRequested_ = false;
    bool shutdownConfirmed_ = false;
    bool showImGuiDemo_ = false;  // Debug builds only; toggled with F12
};
