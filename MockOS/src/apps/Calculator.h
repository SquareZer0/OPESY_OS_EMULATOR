#pragma once

#include "apps/App.h"

class Calculator : public App {
public:
    Calculator();

protected:
    void renderContents() override;
};
