# pe$OS

CSOPESY Semi-Major Output 1: a command-line "OS emulator" with a text marquee
that bounces around the top of the screen like the DVD logo.

## Group developers

- Kimberly Moraca Chong
- Edlynn Rei Encallado
- Michael Stephen Maglente
- Miguel Angelo Ignacio

## Entry point

`main()` is in [main.cpp](main.cpp). The whole program is that one file.

## Requirements

- Windows 10 or later, with a real console window (not a piped/redirected
  terminal — the program reads keystrokes directly via `conio.h`).
- A C++11 or newer compiler. Tested with `g++ (tdm64-1) 9.2.0` from
  TDM-GCC / Dev-C++, under its default settings and with `-std=c++11`,
  `-std=c++14` and `-std=c++17`.

## Build and run

```bash
g++ -o csopesy.exe main.cpp
csopesy.exe
```

Or open `main.cpp` in Dev-C++ and press Compile & Run (F11). No extra
compiler or linker settings are needed.

## Commands

| Command | Description |
|---|---|
| `help` | Lists the available commands |
| `start_marquee` | Starts the bouncing marquee animation |
| `stop_marquee` | Stops the marquee animation |
| `set_text <text>` | Sets the text shown in the marquee |
| `set_speed <ms>` | Sets the marquee refresh rate, in milliseconds per character moved |
| `clear` | Clears the screen and reprints the header |
| `exit` | Terminates the console (Ctrl+C does the same) |

## Notes

- The marquee bounces inside a box at the top of the screen. It moves at an
  angle (never straight sideways or straight up/down), and every time it hits
  a wall it bounces away at a new random angle and changes color. A corner hit
  bounces it back out of the corner. Commands and their output scroll
  underneath the box, so it keeps animating while you type.
- The marquee moves at the same on-screen speed in every direction: one
  character width every `set_speed` milliseconds. It always moves exactly one
  column sideways per frame, so a steeper direction (which also moves up or
  down) waits proportionally longer between frames, up to about 1.4x
  `set_speed` at the steepest angle (45 degrees).
- The box size is measured once at startup (it fills whatever height is left
  after the header and prompt, 9 to 16 rows). Resizing the window while the
  program runs will break the layout.
- Text wider than the box is cut off at the box edge.
- Input is read via a manual polling loop (`POLL_MS` in `main.cpp`) rather
  than blocking `getline`, so both the marquee refresh rate and the input
  polling rate are explicit, tunable values.
