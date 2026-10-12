CSOPESY - pe$OS
===============

This repository holds two CSOPESY projects:

- MockOS/   pe$OS, a mock desktop operating system built with GLFW, OpenGL
            and Dear ImGui.
- Marquee/  pe$OS, a command-line OS emulator with a bouncing text marquee
            (Semi-Major Output 1). See Marquee/README.md.


GROUP MEMBERS
-------------
Kimberly Moraca Chong
Edlynn Rei Encallado
Michael Stephen Maglente
Miguel Angelo Ignacio


MOCKOS: ENTRY POINT
-------------------
main() is in MockOS/src/main.cpp.


MOCKOS: HOW TO BUILD AND RUN
----------------------------
Requires Windows 10 or 11 (64-bit) and Visual Studio 2019, 2022 or 2026
with the "Desktop development with C++" workload. Nothing else needs to be
installed.

1. Open MockOS/MockOS.sln in Visual Studio.
2. Pick the "Release" and "x64" configuration in the toolbar.
3. Press Ctrl+F5 to build and run.

The built program is MockOS/bin/Release/MockOS.exe. It runs on its own from
any folder.

More details, including how to use it: MockOS/README.txt.
