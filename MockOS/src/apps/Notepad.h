#pragma once

#include "apps/App.h"

class Notepad : public App {
public:
    Notepad();

protected:
    void renderContents() override;
};
