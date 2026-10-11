# MockOS plan

## Status

The project builds in Visual Studio, and the whole flow (BIOS → splash →
desktop → PWR → clean exit) works. The desktop has its final look: a pastel,
frosted-glass theme with a top bar and a floating dock. The three apps are
still stubs. Everything marked `PLACEHOLDER (Phase 2)` in the code is waiting
for its owner.

| Requirement | Where | Status |
|---|---|---|
| BIOS screen for a few seconds (memory test, CPU, drives) | `src/screens/BiosScreen.cpp` | Placeholder text |
| Splash screen with animated loading indicator | `src/screens/SplashScreen.cpp` | Done (glass card, loading bar, blinking dots); can be refined |
| Desktop fills the window and follows resizes | `src/screens/Desktop.cpp` | Done |
| Wallpaper | `src/ui/Wallpaper.cpp` | Done (pastel gradient generated in code) |
| Real-time clock (hour and minute) in a fixed corner | `TopBar::renderClock()` | Done (top-right, next to power) |
| PWR button closes the app, no force exit | `TopBar`, `Desktop::renderPowerDialog()`, `Application` | Done |
| Taskbar with 3+ clickable icon buttons | `src/screens/Taskbar.cpp` (the dock) | Done |
| Taskbar shows running apps | `src/screens/Taskbar.cpp` | Done (dot under running apps; front app named in the top bar) |
| Widgets open and close from the buttons, draggable | `App`, `AppManager` | Done |
| Widget 1 and widget 2 | `src/apps/Calculator.cpp`, `src/apps/Notepad.cpp` | Stubs |
| Task Manager table with CPU and memory | `src/apps/TaskManager.cpp` | Stub |
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
- `AppManager` holds the only list of apps. The dock, the top bar, Task
  Manager, and each window's X button all use the same open/closed flag.
- The top bar and the dock are raised above app windows every frame
  (`keepCurrentWindowOnTop()`), and app windows can't be dragged up behind
  the top bar.

**Adding an app:** subclass `App` (see `Calculator`), draw its contents in
`renderContents()`, give it an icon function from `src/ui/Icons.cpp`, and add
it in the `AppManager` constructor. It gets a dock icon and a Task Manager
entry automatically.

**Adding files:** use Solution Explorer → Add → New Item (or Existing Item)
so the file is listed in `MockOS.vcxproj`. Put source files under `src/`.

## Using the theme

Every ImGui widget already uses the pastel theme (`theme::apply()`), so most
app code needs nothing special. For custom touches:

| Need | Use |
|---|---|
| Heading text | `ImGui::PushFont(fonts::semibold(), 18.0f)` … `ImGui::PopFont()` |
| Bigger or smaller text | `ImGui::PushFont(nullptr, size)` … `ImGui::PopFont()` |
| Main action button | `theme::accentButton("Save")` |
| Colors | `theme::TEXT`, `theme::TEXT_MUTED`, `theme::ACCENT` |
| A glass card or panel | `theme::drawGlassPanel(drawList, min, max, rounding)` |
| An icon | add a function to `src/ui/Icons.cpp`, drawn with `ImDrawList` shapes |
| The OS name | `theme::OS_NAME` (`"pe$OS"`) |

## Phase 2 split (suggested)

The desktop and dock are done, so the old "Desktop + Taskbar" share is gone.
Suggested: take Notepad off the Calculator owner and give it to B.

| Owner | Work |
|---|---|
| A | **BIOS**: your own PC's specs, a memory test that counts up to your RAM size, drive detection lines. Optional: read real specs with `__cpuid` and `GlobalMemoryStatusEx`. **Splash**: refine if wanted. |
| B | **Notepad**: `ImGui::InputTextMultiline` on a `std::string` (include `misc/cpp/imgui_stdlib.h`), a menu bar (New, a fake Save), word and character count. |
| C | **Calculator**: display, 4×5 button grid, + − × ÷, decimal point, C/CE, ±, %, divide by zero shows "Error". |
| D | **Task Manager**: `ImGui::BeginTable` with Name, PID, Status, CPU %, Memory (MB); sortable, striped rows, scrolling. Rows are about 10 fake system processes plus every open app (`AppManager::list()`). CPU and memory drift slightly every second. Footer with total usage bars. Bonus: an "End task" button that closes the selected app. |

## Phase 3: polish

- Make the finished apps match the theme (see "Using the theme").
- Images, if anyone adds them, need [stb_image.h](https://github.com/nothings/stb)
  (not included). Build them into the .exe through `MockOS.rc`, like the fonts,
  so the program still works when started from another folder.

## Phase 4: submit

- Update README.txt.
- Build Release, then test on a teammate's PC.
- Submit the GitHub link or a zip.

## Checklist (also works as the demo script)

- [ ] BIOS shows memory test, CPU, and drives for a few seconds, then the splash, then the desktop, with no clicks needed
- [x] The splash has visible animation
- [ ] After resizing or maximizing, the wallpaper fills the window, the top bar spans its width, the dock stays centered, and the clock stays in its corner
- [x] The clock matches the Windows clock and changes when the minute does
- [x] 3+ dock icon buttons; each opens and closes its app; app windows can be dragged
- [x] The dock shows which apps are running and updates when they open and close
- [ ] Task Manager shows a table of processes with CPU and memory values
- [x] PWR closes the app cleanly (exit code 0, process gone from Windows Task Manager)
- [x] The window's X and Alt+F4 don't skip the PWR flow
- [ ] Minimized, the app uses almost no CPU
- [x] Works when started from a different folder

## Notes

- Dear ImGui 1.92 changed fonts: change size with `ImGui::PushFont(font, size)`.
  Tutorials written for older versions use `SetWindowFontScale` or load one
  font per size.
- Glass is a translucent tint, not a real blur: windows behind a glass panel
  show through faintly. Window backgrounds are 80% opaque to keep that subtle.
- In Debug builds, F12 opens `ImGui::ShowDemoWindow()`, which shows every
  widget along with the code for it.
- `imgui.ini` is turned off (`io.IniFilename = nullptr`), so window positions
  always start from the defaults in each app's constructor.
