# MockOS plan

## Status

Phases 0 and 1 are done: the project builds in Visual Studio, and the whole
flow (BIOS → splash → desktop → PWR → clean exit) works with placeholder
screens and apps. Everything marked `PLACEHOLDER (Phase 2)` in the code is
waiting for its owner.

| Requirement | Where | Status |
|---|---|---|
| BIOS screen for a few seconds (memory test, CPU, drives) | `src/screens/BiosScreen.cpp` | Placeholder text |
| Splash screen with animated loading indicator | `src/screens/SplashScreen.cpp` | Placeholder |
| Desktop fills the window and follows resizes | `src/screens/Desktop.cpp` | Done |
| Wallpaper | `Desktop::renderWallpaper()` | Placeholder gradient |
| Real-time clock (hour and minute) in a fixed corner | `Taskbar::renderClock()` | Done |
| PWR button closes the app, no force exit | `Taskbar`, `Desktop::renderPowerDialog()`, `Application` | Done |
| Taskbar with 3+ clickable icon buttons | `src/screens/Taskbar.cpp` | Text buttons, icons to do |
| Widget 1 and widget 2 | `src/apps/Calculator.cpp`, `src/apps/Notepad.cpp` | Stubs |
| Task Manager table with CPU and memory | `src/apps/TaskManager.cpp` | Stub |
| Widgets open and close from the buttons, draggable | `App`, `AppManager` | Done |
| Taskbar shows running apps | `src/screens/Taskbar.cpp` | Basic highlight |
| README.txt with members, how to run, entry file | `README.txt` | Done; update as features land |

## How the code fits together

```
main() → Application::run()
           BIOS ──(4.5 s)──► Splash ──(3 s)──► Desktop ──(PWR confirmed)──► Shutting down ──(1.5 s)──► loop ends → cleanup → return 0
```

- `Application` owns the GLFW window and ImGui, and switches screens. Each
  screen gets the seconds elapsed since it appeared.
- The window's X button and Alt+F4 are redirected to the PWR dialog
  (`Application::windowCloseCallback`). The only way out is PWR, and it
  ends the main loop normally instead of calling `exit()`.
- `AppManager` holds the only list of apps. The taskbar, Task Manager, and
  each window's X button all change the same open/closed flag.
- The taskbar is raised above app windows every frame (`BringWindowToDisplayFront`),
  so a dragged window can't cover it.

**Adding an app:** subclass `App` (see `Calculator`), draw its contents in
`renderContents()`, and add it in the `AppManager` constructor. It gets a
taskbar button and a Task Manager entry automatically.

**Adding files:** use Solution Explorer → Add → New Item (or Existing Item)
so the file is listed in `MockOS.vcxproj`. Put source files under `src/`.

## Phase 2 split

| Owner | Work |
|---|---|
| A | **BIOS**: your own PC's specs, a memory test that counts up to your RAM size, drive detection lines. Optional: read real specs with `__cpuid` and `GlobalMemoryStatusEx`. **Splash**: your own logo (drawn with `ImDrawList` or an image), a loading bar, a blinking ellipsis, a fade into the desktop. |
| B | **Desktop**: your own wallpaper (gradient, pattern, or image). **Taskbar**: icon buttons instead of text, a clearer running-apps display (for example a dot under each open app's icon), clock styling. |
| C | **Calculator**: display, 4×5 button grid, + − × ÷, decimal point, C/CE, ±, %, divide by zero shows "Error". **Notepad**: `ImGui::InputTextMultiline` on a `std::string` (include `misc/cpp/imgui_stdlib.h`), a menu bar (New, a fake Save), word and character count. |
| D | **Task Manager**: `ImGui::BeginTable` with Name, PID, Status, CPU %, Memory (MB); sortable, striped rows, scrolling. Rows are about 10 fake system processes plus every open app (`AppManager::list()`). CPU and memory drift slightly every second. Footer with total usage bars. Bonus: an "End task" button that closes the selected app. |

## Phase 3: polish

- One shared theme (colors, rounding, padding). The default dark style makes
  windows slightly see-through; set `ImGuiCol_WindowBg` alpha to 1.
- A nicer UI font, if wanted (e.g. load `C:\Windows\Fonts\segoeui.ttf` with
  `AddFontFromFileTTF`, keeping the monospace font for the BIOS).
- Icons and images need [stb_image.h](https://github.com/nothings/stb) (not
  included yet). Load assets relative to the .exe's folder
  (`GetModuleFileNameW`), not the current directory, so the program still
  works when started from somewhere else.

## Phase 4: submit

- Update README.txt.
- Build Release, then test on a teammate's PC.
- Submit the GitHub link or a zip.

## Checklist (also works as the demo script)

- [ ] BIOS shows memory test, CPU, and drives for a few seconds, then the splash, then the desktop, with no clicks needed
- [ ] The splash has visible animation
- [ ] After resizing or maximizing, the wallpaper fills the window, the taskbar spans its width, and the clock stays in its corner
- [ ] The clock matches the Windows clock and changes when the minute does
- [ ] 3+ taskbar icon buttons; each opens and closes its app; app windows can be dragged
- [ ] The taskbar shows which apps are running and updates when they open and close
- [ ] Task Manager shows a table of processes with CPU and memory values
- [ ] PWR closes the app cleanly (exit code 0, process gone from Windows Task Manager)
- [ ] The window's X and Alt+F4 don't skip the PWR flow
- [ ] Minimized, the app uses almost no CPU
- [ ] Works when started from a different folder

## Notes

- Dear ImGui 1.92 changed fonts: change size with `ImGui::PushFont(nullptr, size)`.
  Tutorials written for older versions use `SetWindowFontScale` or load one
  font per size.
- In Debug builds, F12 opens `ImGui::ShowDemoWindow()`, which shows every
  widget along with the code for it.
- `imgui.ini` is turned off (`io.IniFilename = nullptr`), so window positions
  always start from the defaults in each app's constructor.
