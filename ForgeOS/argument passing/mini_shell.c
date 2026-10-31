/**
 * ============================================================================
 * PROJECT: Argument Passing & Process Lifecycle Mini-Shell
 * FILE:    mini_shell.c
 * PURPOSE: Demonstrates the journey of command-line arguments:
 *          User Input -> Tokenizer -> argv[] -> fork() -> execvp() -> main()
 * ============================================================================
 * 
 * OS CONCEPTS DEMONSTRATED:
 * 1. Command-line Tokenization & Parsing
 * 2. Memory Management for Argument Vector (argc, argv, NULL-termination)
 * 3. Process Creation using fork()
 * 4. Process Image Replacement using execvp()
 * 5. Parent-Child Process Synchronization using waitpid()
 * 6. Process Termination & Exit Status Decoding (WIFEXITED, WEXITSTATUS, WIFSIGNALED)
 * 7. Shell Built-in Command Handling (exit, help, clear)
 * ============================================================================
 */

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define INITIAL_ARG_CAPACITY 16
#define PROMPT "arg-shell> "

/* Color definitions for enhanced academic visualization */
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_RED     "\033[31m"

/**
 * Structure to hold the tokenized argument vector and count.
 */
typedef struct {
    int argc;       /* Number of arguments */
    char **argv;    /* NULL-terminated array of strings */
} ArgList;

/* Function prototypes */
void print_banner(void);
void print_help(void);
ArgList tokenize_input(const char *input_line);
void free_arglist(ArgList *args);
void print_trace_section(const char *raw_input, const ArgList *args, pid_t shell_pid);
int handle_builtin(const ArgList *args, const char *raw_input);
void execute_external_command(const ArgList *args);

/**
 * main - Shell Read-Eval-Print Loop (REPL)
 */
int main(void) {
    char *line_buffer = NULL;
    size_t line_buffer_size = 0;
    ssize_t bytes_read;
    pid_t shell_pid = getpid();

    print_banner();

    while (1) {
        /* Step 1: Display Shell Prompt */
        printf(COLOR_BOLD COLOR_CYAN "%s" COLOR_RESET, PROMPT);
        fflush(stdout);

        /* Step 2: Read complete line from stdin using getline() */
        bytes_read = getline(&line_buffer, &line_buffer_size, stdin);

        /* Check for EOF (Ctrl+D) or read failure */
        if (bytes_read == -1) {
            if (feof(stdin)) {
                printf("\n" COLOR_YELLOW "[Shell] EOF (Ctrl+D) detected. Terminating shell session." COLOR_RESET "\n");
            } else {
                perror("[Shell] getline error");
            }
            break;
        }

        /* Strip trailing newline character */
        if (bytes_read > 0 && line_buffer[bytes_read - 1] == '\n') {
            line_buffer[bytes_read - 1] = '\0';
            bytes_read--;
        }

        /* Step 3: Handle empty input (or whitespace-only input) safely */
        int is_empty = 1;
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (!isspace((unsigned char)line_buffer[i])) {
                is_empty = 0;
                break;
            }
        }

        if (is_empty) {
            printf(COLOR_YELLOW "[Shell] Empty input detected. No process created. Returning to prompt." COLOR_RESET "\n\n");
            continue;
        }

        /* Step 4: Tokenize raw input into argc and argv[] */
        ArgList args = tokenize_input(line_buffer);

        if (args.argc == 0) {
            printf(COLOR_YELLOW "[Shell] No valid tokens extracted. Returning to prompt." COLOR_RESET "\n\n");
            free_arglist(&args);
            continue;
        }

        /* Step 5: Print Educational Debug/Trace Section */
        print_trace_section(line_buffer, &args, shell_pid);

        /* Step 6: Check for Built-in Commands (exit, help, clear) */
        if (handle_builtin(&args, line_buffer)) {
            free_arglist(&args);
            continue;
        }

        /* Step 7-9: Execute External Command via fork() + execvp() + waitpid() */
        execute_external_command(&args);

        /* Free dynamically allocated token memory for this command iteration */
        free_arglist(&args);
    }

    /* Free line buffer allocated by getline() before exit */
    free(line_buffer);
    printf(COLOR_GREEN "[Shell] Mini-shell exited cleanly. Goodbye!" COLOR_RESET "\n");
    return EXIT_SUCCESS;
}

/**
 * tokenize_input - Custom tokenizer that parses command-line input.
 * 
 * Supports:
 * - Whitespace delimiters (spaces, tabs)
 * - Quoted multi-word strings ("Hello World" or 'Operating Systems')
 * - Strips outer quotes while preserving inner spaces
 * - Gracefully handles unclosed quotes
 * - Dynamically expands argv array as needed
 * - Ensures argv[argc] is NULL
 */
ArgList tokenize_input(const char *input_line) {
    ArgList result;
    result.argc = 0;

    int capacity = INITIAL_ARG_CAPACITY;
    result.argv = malloc(capacity * sizeof(char *));
    if (!result.argv) {
        perror("[Tokenizer] malloc failed");
        exit(EXIT_FAILURE);
    }

    const char *p = input_line;
    char token_buffer[MAX_LINE_LENGTH];
    int token_len = 0;

    while (*p != '\0') {
        /* Skip leading whitespace */
        while (*p != '\0' && isspace((unsigned char)*p)) {
            p++;
        }

        if (*p == '\0') {
            break;
        }

        token_len = 0;

        /* Parse a single argument token (handling quotes) */
        while (*p != '\0' && (!isspace((unsigned char)*p) || token_len == 0)) {
            if (*p == '"' || *p == '\'') {
                char quote_char = *p;
                p++; /* Skip opening quote */

                while (*p != '\0' && *p != quote_char) {
                    if (token_len < MAX_LINE_LENGTH - 1) {
                        token_buffer[token_len++] = *p;
                    }
                    p++;
                }

                if (*p == quote_char) {
                    p++; /* Skip closing quote */
                } else {
                    fprintf(stderr, COLOR_RED "[Tokenizer] Warning: Unclosed quote detected ('%c'). Parsing till end of input." COLOR_RESET "\n", quote_char);
                }
            } else if (isspace((unsigned char)*p)) {
                /* Space outside quotes denotes end of current token */
                break;
            } else {
                /* Regular character */
                if (token_len < MAX_LINE_LENGTH - 1) {
                    token_buffer[token_len++] = *p;
                }
                p++;
            }
        }

        token_buffer[token_len] = '\0';

        /* Store the parsed token */
        if (token_len > 0 || *(p - 1) == '"' || *(p - 1) == '\'') {
            /* Expand capacity if necessary (leaving room for NULL sentinel) */
            if (result.argc >= capacity - 1) {
                capacity *= 2;
                char **new_argv = realloc(result.argv, capacity * sizeof(char *));
                if (!new_argv) {
                    perror("[Tokenizer] realloc failed");
                    free_arglist(&result);
                    exit(EXIT_FAILURE);
                }
                result.argv = new_argv;
            }

            result.argv[result.argc] = strdup(token_buffer);
            if (!result.argv[result.argc]) {
                perror("[Tokenizer] strdup failed");
                free_arglist(&result);
                exit(EXIT_FAILURE);
            }
            result.argc++;
        }
    }

    /* POSIX requirement: argv MUST be NULL-terminated */
    result.argv[result.argc] = NULL;
    return result;
}

/**
 * free_arglist - Safely frees all memory associated with the argument vector.
 */
void free_arglist(ArgList *args) {
    if (args && args->argv) {
        for (int i = 0; i < args->argc; i++) {
            if (args->argv[i]) {
                free(args->argv[i]);
                args->argv[i] = NULL;
            }
        }
        free(args->argv);
        args->argv = NULL;
        args->argc = 0;
    }
}

/**
 * print_trace_section - Prints a structured educational trace showing how
 *                       user input was transformed into an argument vector.
 */
void print_trace_section(const char *raw_input, const ArgList *args, pid_t shell_pid) {
    printf(COLOR_BOLD "------------------------------------------------------------" COLOR_RESET "\n");
    printf(COLOR_BLUE "[Shell]     Shell Process PID : %d" COLOR_RESET "\n", shell_pid);
    printf(COLOR_BLUE "[Shell]     Raw Input Read    : \"%s\"" COLOR_RESET "\n", raw_input);
    printf(COLOR_CYAN "[Tokenizer] Parsed argc       : %d" COLOR_RESET "\n", args->argc);

    for (int i = 0; i < args->argc; i++) {
        printf(COLOR_CYAN "[Tokenizer] argv[%d]           : \"%s\" (Pointer: %p)" COLOR_RESET "\n", 
               i, args->argv[i], (void *)args->argv[i]);
    }
    printf(COLOR_CYAN "[Tokenizer] argv[%d]           : NULL (Sentinel)" COLOR_RESET "\n", args->argc);
    printf(COLOR_BOLD "------------------------------------------------------------" COLOR_RESET "\n");
}

/**
 * handle_builtin - Checks for and executes built-in commands.
 * 
 * Returns: 1 if a built-in was executed, 0 otherwise.
 */
int handle_builtin(const ArgList *args, const char *raw_input) {
    if (args->argc == 0) return 0;

    if (strcmp(args->argv[0], "exit") == 0) {
        printf(COLOR_YELLOW "[Built-in] 'exit' command detected. Shutting down mini-shell." COLOR_RESET "\n");
        return 1;
    }

    if (strcmp(args->argv[0], "help") == 0) {
        print_help();
        printf(COLOR_GREEN "[Shell] Ready for next command." COLOR_RESET "\n\n");
        return 1;
    }

    if (strcmp(args->argv[0], "clear") == 0) {
        /* ANSI escape sequence to clear terminal screen */
        printf("\033[H\033[J");
        return 1;
    }

    return 0;
}

/**
 * execute_external_command - Demonstrates process creation, image replacement,
 *                            and parent-child synchronization.
 * 
 * Flow:
 * 1. Parent calls fork() -> OS creates duplicate child process.
 * 2. Child calls execvp(argv[0], argv) -> OS replaces child memory image with target executable.
 * 3. Target executable's main(int argc, char *argv[]) receives the arguments!
 * 4. Parent calls waitpid() -> blocks until child terminates, then inspects exit status.
 */
void execute_external_command(const ArgList *args) {
    printf(COLOR_MAGENTA "[Shell] Calling fork() to create child process..." COLOR_RESET "\n");

    pid_t pid = fork();

    if (pid < 0) {
        /* fork() failed */
        perror(COLOR_RED "[Shell] ERROR: fork() system call failed" COLOR_RESET);
        return;
    }

    if (pid == 0) {
        /* ================================================================= */
        /* CHILD PROCESS CONTEXT                                            */
        /* ================================================================= */
        printf(COLOR_GREEN "[Child] Process started successfully! (Child PID: %d, Parent PID: %d)" COLOR_RESET "\n", 
               getpid(), getppid());
        printf(COLOR_GREEN "[Child] Preparing to replace process image with executable: \"%s\"" COLOR_RESET "\n", 
               args->argv[0]);
        printf(COLOR_GREEN "[Child] Invoking execvp(\"%s\", argv)..." COLOR_RESET "\n", args->argv[0]);
        fflush(stdout);

        /* 
         * execvp() searches PATH for argv[0] and passes the argument array argv.
         * If execvp succeeds, it NEVER returns because the process image is replaced.
         */
        execvp(args->argv[0], args->argv);

        /* 
         * If execution reaches here, execvp() has FAILED (e.g. command not found).
         * We print an error using perror/strerror and MUST exit immediately using _exit()
         * so the child does NOT continue running the shell loop!
         */
        fprintf(stderr, COLOR_RED "[Child] ERROR: execvp() failed to execute '%s': %s" COLOR_RESET "\n", 
                args->argv[0], strerror(errno));

        /* Use _exit() to avoid flushing parent stdio buffers */
        _exit(127);
    } else {
        /* ================================================================= */
        /* PARENT PROCESS (SHELL) CONTEXT                                   */
        /* ================================================================= */
        printf(COLOR_BLUE "[Parent] fork() returned Child PID: %d" COLOR_RESET "\n", pid);
        printf(COLOR_BLUE "[Parent] Calling waitpid(%d, &status, 0) to synchronize and await child completion..." COLOR_RESET "\n", pid);

        int status;
        pid_t waited_pid = waitpid(pid, &status, 0);

        if (waited_pid == -1) {
            perror(COLOR_RED "[Parent] ERROR: waitpid() failed" COLOR_RESET);
        } else {
            /* Decode the termination status of the child process */
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printf(COLOR_BLUE "[Parent] Child process (PID: %d) terminated normally with exit status: %d" COLOR_RESET "\n", 
                       pid, exit_code);
            } else if (WIFSIGNALED(status)) {
                int term_sig = WTERMSIG(status);
                printf(COLOR_RED "[Parent] Child process (PID: %d) was killed by signal: %d (%s)" COLOR_RESET "\n", 
                       pid, term_sig, strsignal(term_sig));
            } else {
                printf(COLOR_YELLOW "[Parent] Child process (PID: %d) changed state (status: 0x%x)" COLOR_RESET "\n", 
                       pid, status);
            }
        }

        printf(COLOR_MAGENTA "[Shell] Ready for next command." COLOR_RESET "\n\n");
    }
}

/**
 * print_banner - Displays the project banner with academic details.
 */
void print_banner(void) {
    printf(COLOR_BOLD COLOR_GREEN);
    printf("====================================================================\n");
    printf("     OPERATING SYSTEMS & SYSTEMS PROGRAMMING PROJECT\n");
    printf("     Project Title : Argument Passing Mini-Shell\n");
    printf("     Core Concepts : Tokenizer -> fork() -> execvp() -> waitpid()\n");
    printf("====================================================================\n");
    printf(COLOR_RESET);
    printf("Type 'help' for instructions, or run commands (e.g. './argtest hello world').\n");
    printf("Type 'exit' or press Ctrl+D to terminate the shell.\n\n");
}

/**
 * print_help - Displays built-in help and execution lifecycle.
 */
void print_help(void) {
    printf("\n" COLOR_BOLD "================================ MINI-SHELL HELP ================================" COLOR_RESET "\n");
    printf("This mini-shell is an academic tool to demonstrate process creation and argument passing.\n\n");
    printf(COLOR_BOLD "EXECUTION PIPELINE:" COLOR_RESET "\n");
    printf("  1. USER INPUT       : Raw string read from stdin via getline()\n");
    printf("  2. TOKENIZATION     : Parsed into NULL-terminated argv[] array (handles quotes & spaces)\n");
    printf("  3. PROCESS CREATION : fork() creates an identical child process\n");
    printf("  4. IMAGE REPLACEMENT: execvp(argv[0], argv) replaces child image with target binary\n");
    printf("  5. ARGUMENT PASSING : Target main(int argc, char *argv[]) receives the arguments\n");
    printf("  6. SYNCHRONIZATION  : Parent waitpid() waits for child and decodes exit status\n\n");
    printf(COLOR_BOLD "BUILT-IN COMMANDS:" COLOR_RESET "\n");
    printf("  help                : Displays this educational guide\n");
    printf("  clear               : Clears the terminal screen\n");
    printf("  exit                : Exits the mini-shell cleanly\n\n");
    printf(COLOR_BOLD "EXAMPLE COMMANDS:" COLOR_RESET "\n");
    printf("  arg-shell> ./argtest\n");
    printf("  arg-shell> ./argtest hello world 123\n");
    printf("  arg-shell> ./argtest \"Hello Operating Systems World\"\n");
    printf("  arg-shell> ls -l -a\n");
    printf("  arg-shell> echo \"Hello from child process\"\n");
    printf(COLOR_BOLD "=================================================================================" COLOR_RESET "\n\n");
}
