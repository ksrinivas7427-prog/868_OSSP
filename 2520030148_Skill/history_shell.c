#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_HISTORY 10

int main()
{
    /* Dynamic input buffer */
    char *buffer = NULL;
    size_t buffer_size = 0;

    /* Dynamic history array */
    char **history = NULL;

    int history_count = 0;
    int history_capacity = INITIAL_HISTORY;

    /* Allocate initial history array */
    history = malloc(history_capacity * sizeof(char *));

    if (history == NULL)
    {
        printf("Error: Memory allocation failed.\n");
        return 1;
    }

    printf("====================================\n");
    printf("        MY CUSTOM SHELL\n");
    printf("====================================\n");
    printf("Type 'history' to see commands.\n");
    printf("Type 'exit' to quit.\n");
    printf("====================================\n\n");

    /* Main shell loop */
    while (1)
    {
        /* Display shell prompt */
        printf("myshell> ");
        fflush(stdout);

        /* Read input dynamically */
        ssize_t characters = getline(&buffer, &buffer_size, stdin);

        /* Handle Ctrl+D / EOF */
        if (characters == -1)
        {
            printf("\nExiting shell...\n");
            break;
        }

        /* Remove newline character */
        buffer[strcspn(buffer, "\n")] = '\0';

        /* Ignore empty input */
        if (strlen(buffer) == 0)
        {
            continue;
        }

        /* =========================================
           EXIT COMMAND
           ========================================= */

        if (strcmp(buffer, "exit") == 0)
        {
            printf("Exiting shell...\n");
            break;
        }

        /* =========================================
           HISTORY COMMAND
           ========================================= */

        if (strcmp(buffer, "history") == 0)
        {
            printf("\n========== COMMAND HISTORY ==========\n");

            if (history_count == 0)
            {
                printf("No commands in history.\n");
            }
            else
            {
                for (int i = 0; i < history_count; i++)
                {
                    printf("%d  %s\n", i + 1, history[i]);
                }
            }

            printf("=====================================\n\n");

            continue;
        }

        /* =========================================
           DYNAMIC HISTORY RESIZING
           ========================================= */

        if (history_count >= history_capacity)
        {
            printf("[History full - resizing from %d to %d]\n",
                   history_capacity,
                   history_capacity * 2);

            history_capacity *= 2;

            char **temp = realloc(
                history,
                history_capacity * sizeof(char *)
            );

            if (temp == NULL)
            {
                printf("Error: Could not resize history.\n");

                /* Stop shell safely */
                break;
            }

            history = temp;

            printf("[History resized successfully]\n");
        }

        /* =========================================
           ALLOCATE MEMORY FOR NEW COMMAND
           ========================================= */

        history[history_count] = malloc(strlen(buffer) + 1);

        if (history[history_count] == NULL)
        {
            printf("Error: Memory allocation failed.\n");
            break;
        }

        /* Copy command into history */
        strcpy(history[history_count], buffer);

        /* Increase history count */
        history_count++;

        /* =========================================
           PROCESS COMMAND
           ========================================= */

        printf("You entered: %s\n", buffer);
    }

    /* =============================================
       MEMORY CLEANUP
       ============================================= */

    printf("\nCleaning up memory...\n");

    /* Free every stored command */
    for (int i = 0; i < history_count; i++)
    {
        free(history[i]);
        history[i] = NULL;
    }

    /* Free history array */
    free(history);
    history = NULL;

    /* Free dynamic input buffer */
    free(buffer);
    buffer = NULL;

    printf("Memory cleanup complete.\n");

    return 0;
}
