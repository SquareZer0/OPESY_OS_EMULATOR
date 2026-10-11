MockOS (pe$OS)
==============

A mock desktop operating system, pe$OS, built with GLFW, OpenGL and Dear
ImGui. It boots through a BIOS screen and a splash screen into a pastel,
frosted-glass desktop with a top bar, a real-time clock, a dock with three
apps (Calculator, Notepad, Task Manager), and a PWR button that shuts the
program down.


GROUP MEMBERS
-------------
Kimberly Moraca Chong
Edlynn Rei Encallado
Michael Stephen Maglente
Miguel Angelo Ignacio


ENTRY POINT
-----------
main() is in src/main.cpp.


REQUIREMENTS
------------
- Windows 10 or 11, 64-bit.
- Visual Studio 2019, 2022 or 2026 with the "Desktop development with C++"
  workload. Nothing else needs to be installed: Dear ImGui and GLFW are
  included in the external/ folder, and the fonts in assets/fonts/.


HOW TO BUILD AND RUN
--------------------
1. Open MockOS.sln in Visual Studio.
2. Pick the "Release" and "x64" configuration in the toolbar.
3. Press Ctrl+F5 to build and run.

The built program is bin/Release/MockOS.exe. It runs on its own from any
folder: the fonts are built into the .exe, and no DLLs or installs are needed.

To build from the command line instead, open a "Developer Command Prompt
for VS" in this folder and run:

    msbuild MockOS.sln /p:Configuration=Release /p:Platform=x64


HOW TO USE
----------
- The BIOS and splash screens play automatically, then the desktop appears.
- Top bar: the OS name and the app in front on the left. On the right, Wi-Fi
  and sound icons (mock; clicking the speaker toggles mute), the clock (hover
  for the date), and the power button (PWR).
- Dock (the taskbar) at the bottom: click an icon to open its app. A dot under
  an icon means that app is running. Clicking the icon of an app that is
  behind other windows brings it to the front; clicking it while the app is
  in front closes it.
- App windows can be dragged by their title bar and closed with their X.
- The power button shuts pe$OS down after a confirmation. The window's own X
  button and Alt+F4 open the same confirmation instead of closing the app.
- Debug builds only: F12 shows the Dear ImGui demo window.


THIRD-PARTY LIBRARIES AND FONTS
-------------------------------
- Dear ImGui v1.92.9b (MIT License), in external/imgui
- GLFW 3.5.1 (zlib License), in external/glfw
- Poppins font (SIL Open Font License 1.1), in assets/fonts
