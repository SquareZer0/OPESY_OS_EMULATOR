# pe$OS

CSOPESY Semi-Major Output 1: a command-line "OS emulator" with a text marquee.

## Group developers

- Kimberly Moraca Chong
- Edlynn Rei Encallado
- Michael Stephen Maglente
- Miguel Angelo Ignacio

## Entry point

`main()` is in [main.cpp](main.cpp). The whole program is that one file.

## Requirements

- Windows, with a real console window (not a piped/redirected terminal — the
  program reads keystrokes directly via `conio.h` and needs an actual TTY).
- A C++17 compiler. Tested with `g++ (tdm64-1) 9.2.0` from TDM-GCC / Dev-C++.

## Build and run

```bash
g++ -std=c++17 -o csopesy.exe main.cpp
csopesy.exe
```

Or build and run it from Dev-C++ (or any IDE) via Run/Debug.

## Commands

| Command | Description |
|---|---|
| `help` | Lists the available commands |
| `start_marquee` | Starts the marquee animation |
| `stop_marquee` | Stops the marquee animation |
| `set_text <text>` | Sets the text shown in the marquee |
| `set_speed <ms>` | Sets the marquee refresh rate, in milliseconds |
| `clear` | Clears the screen and reprints the header |
| `exit` | Terminates the console |

## Notes

- The marquee is drawn on a permanently reserved row 1, so it keeps animating
  while you type commands or read their output below it.
- Input is read via a manual polling loop (`POLL_MS` in `main.cpp`) rather
  than blocking `getline`, so both the marquee refresh rate and the input
  polling rate are explicit, tunable constants.
