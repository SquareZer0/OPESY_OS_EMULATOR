# pe$OS

CSOPESY Semi-Major Output 1: a command-line "OS emulator" with a text marquee
that bounces around the top of the screen like the DVD logo.

Homework 2 adds a multi-threaded first-come-first-serve (FCFS) CPU scheduler
with 4 cores, the `screen -ls` command, and a temporary per-process `print`
log.

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
| `screen -ls` | Lists the running and finished processes |
| `start_marquee` | Starts the bouncing marquee animation |
| `stop_marquee` | Stops the marquee animation |
| `set_text <text>` | Sets the text shown in the marquee |
| `set_speed <ms>` | Sets the marquee refresh rate, in milliseconds per character moved |
| `clear` | Clears the screen and reprints the header |
| `exit` | Terminates the console (Ctrl+C does the same) |

## FCFS scheduler

- **Threads:** one scheduler thread plus one worker thread per CPU core
  (`NUM_CORES = 4`), plus the existing marquee thread and the input loop on
  the main thread.
- **Startup:** `NUM_PROCESSES = 10` processes (`process01` to `process10`),
  each with `PRINTS_PER_PROCESS = 100` print commands, are created and queued
  in that order as soon as the emulator starts. No command is needed to begin.
- **Scheduling:** the scheduler thread gives the process at the front of the
  ready queue to an idle core. A process runs on its core until all of its
  print commands are done (FCFS is non-preemptive), then the core reports
  itself idle and the scheduler assigns the next process. With 10 processes
  and 4 cores they run in waves: 1-4, then 5-8, then 9-10.
- **Print command:** each print takes `PRINT_DELAY_MS = 100` ms on the core,
  so a process lasts about 10 seconds. This keeps processes visibly in the
  "running" list while you type `screen -ls` every 1-2 seconds.
- **`screen -ls`:** prints each running process (start time, core, commands
  done / total) and each finished process (finish time, `Finished`, total /
  total), in the layout of the reference photo. Processes still waiting in the
  ready queue are not listed until a core picks them up.

### Print log files (homework only)

When `WRITE_PRINT_LOGS` is `true` (the default), each process writes its print
commands to `logs/<process name>.txt`, so 10 processes produce 10 files in a
`logs` folder next to the executable (it is created automatically, and files
from a previous run are overwritten). Each file looks like:

```
Process name: process01
Logs:

(10/17/2026 05:03:59AM) Core:0 "Hello world from process01!"
```

Each line carries the time the print ran and the core that ran it. **Set
`WRITE_PRINT_LOGS` to `false` in `main.cpp` for the machine project**, since
file output slows down the scheduler.

### Test case

1. Run the program from the IDE (the 10 processes start automatically).
2. Type `screen -ls` every 1-2 seconds until all 10 processes appear under
   "Finished processes" (about 30 seconds).
3. Type `exit`.
4. Zip the `logs` folder for submission.

## Marquee notes

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