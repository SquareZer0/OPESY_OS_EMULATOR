#pragma once

#include "apps/App.h"

// A mock File Explorer: a sidebar of folders, each holding a few made-up files.
class FileExplorer : public App {
public:
    FileExplorer();

protected:
    void renderContents() override;

private:
    void renderSidebar(float height);
    void renderFolder();

    int folder_ = 0;         // index of the folder shown on the right
    int selectedFile_ = -1;  // highlighted row in that folder, or -1
};
