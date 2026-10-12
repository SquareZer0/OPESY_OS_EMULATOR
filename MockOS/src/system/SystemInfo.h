#pragma once

#include <string>
#include <vector>

// One physical drive, as reported by Windows.
struct DriveInfo {
    std::string bus;     // "NVMe", "SATA", "USB", ...
    std::string model;   // for example "ADATA LEGEND 860"
    double sizeGB = 0.0; // 0 if Windows didn't report it
};

// Hardware details of the PC the program runs on, for the BIOS screen.
// Any field Windows couldn't provide is left empty or zero.
struct SystemInfo {
    std::string cpuName;
    int cpuMHz = 0;
    int cores = 0;
    int threads = 0;
    unsigned long long memoryKB = 0;  // installed RAM
    std::string boardMaker;
    std::string boardModel;
    std::string gpuName;
    std::vector<DriveInfo> drives;
};

// Reads the details from Windows. Needs no admin rights. Call it after the
// OpenGL context exists, since the graphics card name comes from OpenGL.
SystemInfo querySystemInfo();
