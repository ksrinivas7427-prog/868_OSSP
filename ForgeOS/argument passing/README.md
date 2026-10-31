# Argument Passing & Process Lifecycle Mini-Shell

> **Course:** Operating Systems & Systems Programming  
> **Topic:** Process Management, Argument Parsing, and Process Image Replacement  
> **Language:** C (POSIX / Linux Standard)  
> **Key System Calls:** `fork()`, `execvp()`, `waitpid()`, `_exit()`, `getline()`

---

## 1. Project Objective

The primary objective of this project is to build an educational **Linux Mini-Shell** that demystifies how command-line arguments travel from interactive user input through the operating system kernel and into the `main(int argc, char *argv[])` function of a target executable.

Instead of hiding the underlying mechanics behind abstractions, this mini-shell provides **transparent, color-coded execution tracing** at every stage of the POSIX process lifecycle.

```
+-----------------------------------------------------------------------------------+
|                               EXECUTION PIPELINE                                  |
+-----------------------------------------------------------------------------------+
|  [1] USER INPUT         : "echo \"Hello World\""                                  |
|         |                                                                         |
|         v                                                                         |
|  [2] TOKENIZER          : Splits string into argv[0]="echo", argv[1]="Hello World"|
|         |                                                                         |
|         v                                                                         |
|  [3] fork()             : Duplicates shell process -> Child (PID: X) created      |
|         |                                                                         |
|         v                                                                         |
|  [4] execvp()           : Replaces Child image with ELF binary of target program  |
|         |                                                                         |
|         v                                                                         |
|  [5] main(argc, argv)   : Kernel & C runtime (crt0) set up target stack frame     |
|         |                                                                         |
|         v                                                                         |
|  [6] CHILD EXITS        : Target terminates with exit status code                 |
|         |                                                                         |
|         v                                                                         |
|  [7] waitpid()          : Parent Shell reaps child, prevents zombie, decodes status|
+-----------------------------------------------------------------------------------+
```

---

## 2. Theoretical Architecture & OS Mechanics

### A. Tokenization and Memory Management
When the user enters a command line (e.g., `./argtest -a "Operating Systems"`):
1. `getline()` captures the entire input string from standard input into a dynamically allocated buffer.
2. The custom **Tokenizer** scans the string character-by-character:
   - Skips extraneous whitespace.
   - Identifies argument boundaries.
   - Handles single (`'...'`) and double (`"..."`) quotation marks, stripping surrounding quotes while preserving internal whitespace.
   - Duplicates each token into heap memory (`strdup()`).
   - Builds a dynamic `argv` array of `char *` pointers.
   - Appends a mandatory `NULL` pointer at `argv[argc]` to comply with POSIX standards.

### B. Process Creation (`fork()`)
```c
pid_t pid = fork();
```
- The `fork()` system call duplicates the calling process.
- The operating system creates a child process with a unique PID and copies the parent's memory layout (using Copy-On-Write / COW optimization).
- **Return Values:**
  - In the Child process: `fork()` returns `0`.
  - In the Parent process: `fork()` returns the child's `PID` (> 0).
  - On failure: `fork()` returns `-1` and sets `errno`.

### C. Process Image Replacement (`execvp()`)
```c
execvp(argv[0], argv);
```
- In the child process, `execvp()` searches the system `PATH` (or takes a relative/absolute path) for the executable named `argv[0]`.
- **Image Replacement:** The kernel releases the child's existing text, data, heap, and stack segments, loading the new ELF executable binary into memory.
- **Stack Setup:** The kernel places `argc`, the `argv[]` pointer table, the environment variables (`envp[]`), and the argument strings at the top of the child's new user stack.
- If `execvp()` succeeds, **it never returns**.
- If `execvp()` fails (e.g., file not found or permission denied), it returns `-1`, prints an error message, and the child calls `_exit(127)` immediately to prevent duplicate shell execution.

### D. Parent-Child Synchronization (`waitpid()`)
```c
waitpid(pid, &status, 0);
```
- The parent shell halts execution until the child process changes state (terminates).
- Prevents **zombie processes** (processes whose execution is finished but remain in the process table).
- Inspects the exit condition:
  - `WIFEXITED(status)`: Evaluates to true if the child exited normally (via `exit()` or `return`).
  - `WEXITSTATUS(status)`: Extracts the integer exit code (e.g., 0, 127).
  - `WIFSIGNALED(status)`: Evaluates to true if the child was killed by a signal (e.g., SIGSEGV, SIGINT).
  - `WTERMSIG(status)`: Returns the signal number that caused the termination.

---

## 3. Memory Layout of Argument Passing

When `execvp()` invokes `main(int argc, char *argv[])`, the memory layout on the user stack is organized as follows:

```
+-------------------------------------------------------------+
| Higher Memory Addresses (Bottom of Stack)                   |
|-------------------------------------------------------------|
| "Hello World\0"          <- String buffer 2                 |
| "./argtest\0"            <- String buffer 1                 |
|-------------------------------------------------------------|
| NULL                     <- argv[2] (Sentinel pointer)       |
| Pointer to "./argtest"   <- argv[0]                         |
| Pointer to "Hello World" <- argv[1]                         |
|-------------------------------------------------------------|
| argv (char **)           <- Pointer to array of pointers    |
| argc = 2 (int)           <- Argument count                  |
| Return Address to crt0                                      |
|-------------------------------------------------------------|
| Lower Memory Addresses (Stack grows downward)               |
+-------------------------------------------------------------+
```

---

## 4. File Structure

```
.
├── Makefile        # Compilation and build automation rules
├── mini_shell.c    # Mini-shell with tokenizer, fork, execvp, waitpid & built-ins
├── argtest.c       # Target program demonstrating argc, argv & memory verification
├── README.md       # Comprehensive architectural & debugging documentation
└── TESTING.md      # Detailed test matrix with inputs, outputs & OS concepts
```

---

## 5. Prerequisites & Ubuntu Installation

To install GCC, GDB, and standard build tools on Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y build-essential gcc gdb make
```

---

## 6. Compilation & Execution

### Compile all binaries:
```bash
make
```
*Equivalent command:*
```bash
gcc -Wall -Wextra -g -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -o mini_shell mini_shell.c
gcc -Wall -Wextra -g -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -o argtest argtest.c
```

### Run the Mini-Shell:
```bash
make run
# or directly:
./mini_shell
```

### Clean compiled binaries:
```bash
make clean
```

---

## 7. Interactive GDB Debugging Guide

Debugging multi-process programs with `fork()` and `execvp()` requires configuring GDB's fork following mode.

### A. Following Child Process across `fork()` and `execvp()`
By default, GDB stays attached to the parent process after a `fork()`. To follow the child into the new executable image:

```bash
gdb ./mini_shell
```

Inside GDB:
```gdb
# 1. Set breakpoints at key OS lifecycle points
(gdb) break main
(gdb) break fork
(gdb) break execvp
(gdb) break waitpid

# 2. Configure GDB to follow the child process upon fork()
(gdb) set follow-fork-mode child

# 3. Configure GDB to stay attached when execvp replaces the process image
(gdb) set follow-exec-mode new

# 4. Start execution
(gdb) run

# 5. When prompted by arg-shell>, type:
arg-shell> ./argtest "Hello OS"

# 6. GDB stops at fork() in mini_shell.c
(gdb) print args.argc
(gdb) print args.argv[0]
(gdb) print args.argv[1]
(gdb) continue

# 7. GDB stops at execvp() inside the child process
(gdb) print argv[0]
(gdb) print argv[1]
(gdb) print argv[2]
(gdb) continue

# 8. GDB loads the new symbol table for ./argtest automatically!
(gdb) break main
(gdb) continue

# 9. Inspect argc/argv received in argtest's main()
(gdb) info locals
(gdb) print argc
(gdb) print argv[0]
(gdb) print argv[1]
(gdb) print argv[2]

# 10. Finish execution
(gdb) continue
(gdb) quit
```

### B. Explanation of GDB Settings
| GDB Command | Function / Educational Explanation |
|---|---|
| `set follow-fork-mode child` | Tells the debugger to detach from parent and switch inspection to the newly spawned child process immediately after `fork()`. |
| `set follow-fork-mode parent` | (Default) Keeps debugger attached to the parent process while the child executes independently. |
| `set follow-exec-mode new` | Keeps debugging the new executable image after `execvp()` replaces the memory space. |
| `info inferiors` | Lists all active processes managed by GDB. |

---

## 8. Sample Trace Output

```text
====================================================================
     OPERATING SYSTEMS & SYSTEMS PROGRAMMING PROJECT
     Project Title : Argument Passing Mini-Shell
     Core Concepts : Tokenizer -> fork() -> execvp() -> waitpid()
====================================================================
Type 'help' for instructions, or run commands (e.g. './argtest hello world').
Type 'exit' or press Ctrl+D to terminate the shell.

arg-shell> ./argtest "Operating Systems" 2026
------------------------------------------------------------
[Shell]     Shell Process PID : 32410
[Shell]     Raw Input Read    : "./argtest "Operating Systems" 2026"
[Tokenizer] Parsed argc       : 3
[Tokenizer] argv[0]           : "./argtest" (Pointer: 0x55d1a49802a0)
[Tokenizer] argv[1]           : "Operating Systems" (Pointer: 0x55d1a49802c0)
[Tokenizer] argv[2]           : "2026" (Pointer: 0x55d1a49802e0)
[Tokenizer] argv[3]           : NULL (Sentinel)
------------------------------------------------------------
[Shell] Calling fork() to create child process...
[Parent] fork() returned Child PID: 32411
[Parent] Calling waitpid(32411, &status, 0) to synchronize and await child completion...
[Child] Process started successfully! (Child PID: 32411, Parent PID: 32410)
[Child] Preparing to replace process image with executable: "./argtest"
[Child] Invoking execvp("./argtest", argv)...

================================================================
                   ARGUMENT RECEIVER PROGRAM                    
================================================================
[Target] Process PID       : 32411
[Target] Parent Process PID: 32410
[Target] argc (Count)      : 3
[Target] argv Base Address : 0x7ffd52119938
----------------------------------------------------------------
[Target] argv[0] = "./argtest" (Address: 0x7ffd5211a76c) [Program Name/Path]
[Target] argv[1] = "Operating Systems" (Address: 0x7ffd5211a776)
[Target] argv[2] = "2026" (Address: 0x7ffd5211a788)
----------------------------------------------------------------
[Target] argv[3] = NULL (Address: (nil)) [Verified POSIX Sentinel]
================================================================

[Parent] Child process (PID: 32411) terminated normally with exit status: 0
[Shell] Ready for next command.
```
