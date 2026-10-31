/**
 * ============================================================================
 * PROJECT: Argument Passing & Process Lifecycle Mini-Shell
 * FILE:    argtest.c
 * PURPOSE: Target program specifically designed to demonstrate how
 *          main(int argc, char *argv[]) receives arguments from execvp().
 * ============================================================================
 * 
 * OS CONCEPTS DEMONSTRATED:
 * 1. argc: Argument Count passed by the OS / C runtime startup code (crt0)
 * 2. argv: Argument Vector (array of pointers to null-terminated strings)
 * 3. argv[0]: Program invocation name
 * 4. argv[argc]: POSIX requirement guaranteed to be a NULL pointer sentinel
 * 5. Memory layout of arguments on the user stack
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_MAGENTA "\033[35m"

int main(int argc, char *argv[]) {
    printf("\n" COLOR_BOLD COLOR_GREEN "================================================================" COLOR_RESET "\n");
    printf(COLOR_BOLD COLOR_GREEN "                   ARGUMENT RECEIVER PROGRAM                    " COLOR_RESET "\n");
    printf(COLOR_BOLD COLOR_GREEN "================================================================" COLOR_RESET "\n");

    /* Print Process IDs to prove we are running in the child process image */
    printf(COLOR_CYAN "[Target] Process PID       : %d" COLOR_RESET "\n", getpid());
    printf(COLOR_CYAN "[Target] Parent Process PID: %d" COLOR_RESET "\n", getppid());
    printf(COLOR_YELLOW "[Target] argc (Count)      : %d" COLOR_RESET "\n", argc);
    printf(COLOR_MAGENTA "[Target] argv Base Address : %p" COLOR_RESET "\n", (void *)argv);
    printf("----------------------------------------------------------------\n");

    /* Iterate through all arguments */
    for (int i = 0; i < argc; i++) {
        if (i == 0) {
            printf(COLOR_BOLD "[Target] argv[%d] = \"%s\"" COLOR_RESET " (Address: %p) [Program Name/Path]\n", 
                   i, argv[i], (void *)argv[i]);
        } else {
            printf(COLOR_BOLD "[Target] argv[%d] = \"%s\"" COLOR_RESET " (Address: %p)\n", 
                   i, argv[i], (void *)argv[i]);
        }
    }

    /* Verify POSIX NULL-termination of argv array */
    printf("----------------------------------------------------------------\n");
    if (argv[argc] == NULL) {
        printf(COLOR_GREEN "[Target] argv[%d] = NULL (Address: %p) [Verified POSIX Sentinel]" COLOR_RESET "\n", 
               argc, (void *)argv[argc]);
    } else {
        printf(COLOR_MAGENTA "[Target] WARNING: argv[%d] is NOT NULL!" COLOR_RESET "\n", argc);
    }
    printf(COLOR_BOLD COLOR_GREEN "================================================================" COLOR_RESET "\n\n");

    return 0;
}
