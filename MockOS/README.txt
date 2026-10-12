MockOS (pe$OS)
==============

A mock desktop operating system, pe$OS, built with GLFW, OpenGL and Dear
ImGui. It boots through a BIOS screen, a loading screen and a splash screen
into a pastel, frosted-glass desktop with a top bar, a real-time clock, a
dock with five apps (Chrome, File Explorer, Calculator, Notepad, Task
Manager), and a PWR button that shuts the program down.


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
- The BIOS, loading and splash screens play automatically (about 9.5
  seconds), then the desktop appears.
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


APPS
----
Chrome
- A mock browser showing the Google page. You can type in the "Ask Google"
  box, but nothing loads: Enter, the address bar and the toolbar buttons do
  nothing.

File Explorer
- Click a folder in the sidebar (Downloads, Documents, Music, Pictures,
  Videos, This PC, Network) to see what's in it. The files are made up and
  can't be opened.

Calculator
- Works like the Windows basic calculator: each operation runs when the next
  one is pressed, so 5 + 3 x 2 = gives 16.
- C clears everything, CE clears the current number, +/- flips the sign, and
  % takes a percentage (50 + 10 % = gives 55). Dividing by zero shows "Error";
  press C, CE or a number to start over.
- While the Calculator is in front, the keyboard works too: number keys,
  + - * / % and ., Enter for =, Esc for C, Delete for CE, and Backspace to
  delete the last digit typed.

Notepad
- Type in the box; long lines wrap at the edge of the window. The line at the
  bottom counts words and characters.
- File > New clears the text. File > Save is a mock: it shows "Saved" but
  writes nothing, so the text is gone once Notepad is closed.

Task Manager
- Lists mock system processes plus every open app, with PID, status, CPU and
  memory. The CPU and memory values are made up and change every second.
- Click a column header to sort by it; click it again to reverse the order.
- Select an app's row and press End task to close that app. System processes
  can't be ended. An app that is opened again gets a new PID.
- Only the Processes page is built; the other sidebar pages are greyed out.


THIRD-PARTY LIBRARIES AND FONTS
-------------------------------
- Dear ImGui v1.92.9b (MIT License), in external/imgui
- GLFW 3.5.1 (zlib License), in external/glfw
- Poppins font (SIL Open Font License 1.1), in assets/fonts
