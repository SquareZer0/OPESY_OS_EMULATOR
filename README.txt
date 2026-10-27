pe$OS - CSOPESY Process Scheduler Emulator
==========================================

Group members
-------------
Kimberly Moraca Chong
Edlynn Rei Encallado
Michael Stephen Maglente
Miguel Angelo Ignacio

Entry point
-----------
main() is in main.cpp.

Files
-----
main.cpp            Command line: main menu, process screens, commands (main() is here)
Scheduler.h/.cpp    CPU cores, ready queue, FCFS / Round Robin, process generation, screen -ls report
SimProcess.h/.cpp   A simulated process: its program, variables, logs, and instruction execution
Instruction.h/.cpp  PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR, and random program generation
Config.h/.cpp       Reading and checking config.txt
Console.h/.cpp      Console helpers: colors, clearing the screen, timestamps
config.txt          Settings read by 'initialize'
csopesy.dev         Dev-C++ project file

How to run
----------
Requirements: Windows 10 or later, and Dev-C++ 6.3 (Embarcadero, with TDM-GCC 9.2)
or any g++ that supports C++11 and std::thread.

Dev-C++:
  1. File > Open Project or File, and open csopesy.dev.
  2. Press F11 (Compile & Run).
  config.txt must be in the same folder as csopesy.dev, since the program runs from there.

Command line:
  g++ -o csopesy.exe main.cpp Scheduler.cpp SimProcess.cpp Instruction.cpp Config.cpp Console.cpp
  csopesy.exe

config.txt
----------
One "key value" setting per line. All keys are required.
  num-cpu             Number of CPU cores, 1-128
  scheduler           fcfs or rr (quotes optional)
  quantum-cycles      Round robin time slice in CPU ticks, 1 to 2^32
  batch-process-freq  scheduler-start creates one process every this many ticks, 1 to 2^32
  min-ins, max-ins    Instructions per process, 1 to 2^32 (min-ins <= max-ins)
  delay-per-exec      Extra ticks a core stays busy after each instruction, 0 to 2^32
One CPU tick is 10 ms of real time.

Commands
--------
Main menu (before 'initialize', only 'initialize' and 'exit' work):
  initialize          Reads config.txt and starts the CPU cores
  screen -s <name>    Creates a process and opens its screen
  screen -r <name>    Opens the screen of a process that hasn't finished
  screen -ls          CPU utilization, cores used/available, running and finished processes
  scheduler-start     Starts creating dummy processes (p01, p02, ...) every batch-process-freq ticks
  scheduler-stop      Stops creating dummy processes; existing ones keep running
  report-util         Saves the screen -ls output to csopesy-log.txt
  help                Lists the commands
  clear               Clears the screen
  exit                Shuts down the emulator

Inside a process screen:
  process-smi         Process name, ID, logs, and progress (or "Finished!" once done)
  exit                Returns to the main menu

Notes
-----
- The CPU cores run from 'initialize' until 'exit'. scheduler-start and scheduler-stop
  only control generating new processes; processes made with screen -s run either way.
- SLEEP(X) takes the process off its core for X ticks so another process can run.
  delay-per-exec keeps the core busy (the process stays on it).
- Variables are uint16, so results are clamped to 0-65535. Undeclared variables read as 0.
- A process's instruction count includes every repetition of a FOR loop's body.
  FOR loops nest up to 3 levels deep.
