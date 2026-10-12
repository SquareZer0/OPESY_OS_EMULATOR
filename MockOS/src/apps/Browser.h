#pragma once

#include <string>

#include "apps/App.h"

// A mock Chrome window: the toolbar and Google page look real, but nothing
// navigates. Only the search box takes typing.
class Browser : public App {
public:
    Browser();

protected:
    void renderContents() override;
    ImGuiWindowFlags windowFlags() const override {
        return ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    }

private:
    void renderToolbar(float left, float right, float top);
    void renderPage(const ImVec2& min, const ImVec2& max);

    std::string query_;
};
