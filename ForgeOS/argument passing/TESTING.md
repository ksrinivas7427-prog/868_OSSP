# Comprehensive Testing Matrix & Verification Guide

This document details test scenarios for the **Argument Passing Mini-Shell**, covering argument variations, quote handling, standard utilities, error scenarios, and built-in commands.

---

## Testing Summary Table

| Test # | Command Input | Parsed `argc` | Parsed `argv` Elements | Expected Behavior | Core OS Concept |
|:---:|:---|:---:|:---|:---|:---|
| **1** | `./argtest` | 1 | `[0]="./argtest"`, `[1]=NULL` | Executes target program with zero user arguments | Binary invocation via `argv[0]`, default `argc=1` |
| **2** | `./argtest hello` | 2 | `[0]="./argtest"`, `[1]="hello"`, `[2]=NULL` | Passes single argument string to target binary | Single parameter passing via `argv[1]` |
| **3** | `./argtest hello world 123` | 4 | `[0]="./argtest"`, `[1]="hello"`, `[2]="world"`, `[3]="123"`, `[4]=NULL` | Passes multiple heterogeneous arguments | Multi-token vector construction and stack layout |
| **4** | `./argtest -a -l -v` | 4 | `[0]="./argtest"`, `[1]="-a"`, `[2]="-l"`, `[3]="-v"`, `[4]=NULL` | Passes CLI options/flags seamlessly | Command-line option passing and delimiter parsing |
| **5** | `./argtest "Hello World"` | 2 | `[0]="./argtest"`, `[1]="Hello World"`, `[2]=NULL` | Preserves space inside quotation marks, strips outer quotes | Quoted tokenization, single multi-word `argv` element |
| **6** | `echo "Operating Systems Project"` | 2 | `[0]="echo"`, `[1]="Operating Systems Project"`, `[2]=NULL` | Searches `PATH` for `echo`, replaces child image, prints output | `PATH` lookup by `execvp()`, system utility invocation |
| **7** | `invalid_command_123` | 1 | `[0]="invalid_command_123"`, `[1]=NULL` | `execvp()` fails, prints error, child exits with 127, parent reaps status | Error handling in `execvp()`, child `_exit(127)`, `WEXITSTATUS` decoding |
| **8** | *(Empty Enter / Whitespace)* | 0 | `NULL` | Detects empty input, avoids `fork()`, prompts again | Defensive shell input validation |
| **9 (Bonus)**| `./argtest 'Single Quote' "Double Quote"` | 3 | `[0]="./argtest"`, `[1]="Single Quote"`, `[2]="Double Quote"`, `[3]=NULL` | Supports mixed quotation styles | Custom Lexical Parsing |
| **10 (Bonus)**| `help` / `exit` | 1 | `[0]="help"` or `[0]="exit"` | Executes in parent without calling `fork()` | Shell Built-in Command vs External Binary execution |

---

## Detailed Test Case Walkthroughs

### Test Case 1: Zero User Arguments
- **Input:**
  ```text
  arg-shell> ./argtest
  ```
- **Expected Tokenizer Output:**
  ```text
  [Tokenizer] Parsed argc       : 1
  [Tokenizer] argv[0]           : "./argtest"
  [Tokenizer] argv[1]           : NULL
  ```
- **Expected Target Output:**
  ```text
  [Target] argc (Count)      : 1
  [Target] argv[0] = "./argtest" [Program Name/Path]
  [Target] argv[1] = NULL [Verified POSIX Sentinel]
  ```
- **OS Concept:** `argc` is always at least 1 in C/POSIX programs; `argv[0]` holds the path/name used to invoke the program.

---

### Test Case 2: Single Argument
- **Input:**
  ```text
  arg-shell> ./argtest hello
  ```
- **Expected Tokenizer Output:**
  ```text
  [Tokenizer] Parsed argc       : 2
  [Tokenizer] argv[0]           : "./argtest"
  [Tokenizer] argv[1]           : "hello"
  [Tokenizer] argv[2]           : NULL
  ```
- **Expected Target Output:**
  ```text
  [Target] argc (Count)      : 2
  [Target] argv[0] = "./argtest"
  [Target] argv[1] = "hello"
  [Target] argv[2] = NULL
  ```
- **OS Concept:** Argument count increments to 2. The string `"hello"` is copied to the child stack during `execvp()`.

---

### Test Case 3: Multiple Arguments
- **Input:**
  ```text
  arg-shell> ./argtest hello world 123
  ```
- **Expected Tokenizer Output:**
  ```text
  [Tokenizer] Parsed argc       : 4
  [Tokenizer] argv[0]           : "./argtest"
  [Tokenizer] argv[1]           : "hello"
  [Tokenizer] argv[2]           : "world"
  [Tokenizer] argv[3]           : "123"
  [Tokenizer] argv[4]           : NULL
  ```
- **Expected Target Output:**
  ```text
  [Target] argc (Count)      : 4
  [Target] argv[0] = "./argtest"
  [Target] argv[1] = "hello"
  [Target] argv[2] = "world"
  [Target] argv[3] = "123"
  [Target] argv[4] = NULL
  ```
- **OS Concept:** Numerical strings (`"123"`) are passed as character arrays; the C runtime maintains the order of arguments in the pointer vector.

---

### Test Case 4: Multiple Flags/Options
- **Input:**
  ```text
  arg-shell> ./argtest -a -l -v
  ```
- **Expected Tokenizer Output:**
  ```text
  [Tokenizer] Parsed argc       : 4
  [Tokenizer] argv[0]           : "./argtest"
  [Tokenizer] argv[1]           : "-a"
  [Tokenizer] argv[2]           : "-l"
  [Tokenizer] argv[3]           : "-v"
  [Tokenizer] argv[4]           : NULL
  ```
- **OS Concept:** Demonstrates how UNIX command-line switches are parsed by the shell tokenizer and delivered into program parameters.

---

### Test Case 5: Quoted Multi-Word Arguments
- **Input:**
  ```text
  arg-shell> ./argtest "Hello World"
  ```
- **Expected Tokenizer Output:**
  ```text
  [Tokenizer] Parsed argc       : 2
  [Tokenizer] argv[0]           : "./argtest"
  [Tokenizer] argv[1]           : "Hello World"
  [Tokenizer] argv[2]           : NULL
  ```
- **Expected Target Output:**
  ```text
  [Target] argc (Count)      : 2
  [Target] argv[0] = "./argtest"
  [Target] argv[1] = "Hello World"
  [Target] argv[2] = NULL
  ```
- **OS Concept:** Quotes group multiple whitespace-separated words into a single `argv[1]` element. Outer quotation characters are stripped.

---

### Test Case 6: Standard Linux Utility (`echo`)
- **Input:**
  ```text
  arg-shell> echo "Operating Systems Project"
  ```
- **Expected Output:**
  ```text
  [Shell] Calling fork() to create child process...
  [Child] Process started successfully! (Child PID: 32550, Parent PID: 32549)
  [Child] Invoking execvp("echo", argv)...
  Operating Systems Project
  [Parent] Child process (PID: 32550) terminated normally with exit status: 0
  ```
- **OS Concept:** `execvp()` performs environment `PATH` resolution to find `/bin/echo` or `/usr/bin/echo` without requiring an explicit relative path.

---

### Test Case 7: Invalid Command / Execution Failure
- **Input:**
  ```text
  arg-shell> invalid_command_123
  ```
- **Expected Output:**
  ```text
  [Shell] Calling fork() to create child process...
  [Child] Process started successfully! (Child PID: 32560, Parent PID: 32549)
  [Child] Invoking execvp("invalid_command_123", argv)...
  [Child] ERROR: execvp() failed to execute 'invalid_command_123': No such file or directory
  [Parent] Child process (PID: 32560) terminated normally with exit status: 127
  [Shell] Ready for next command.
  ```
- **OS Concept:** When `execvp()` fails, the child must immediately call `_exit(127)`. The parent uses `waitpid()` and `WEXITSTATUS(status)` to detect and report the error code.

---

### Test Case 8: Empty Input & Whitespace
- **Input:**
  ```text
  arg-shell>     [Press Enter]
  ```
- **Expected Output:**
  ```text
  [Shell] Empty input detected. No process created. Returning to prompt.
  arg-shell>
  ```
- **OS Concept:** Prevents extraneous `fork()` system calls when no command is provided, avoiding system resource wastage.
