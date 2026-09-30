*
         * Extract PID from the message if present.
         */
        if (strncmp(buffer, "PID:", 4) == 0) {

            char temp[256];
            char *colon;
            pid_t client_pid;

            strcpy(temp, buffer);

            colon = strchr(temp + 4, ':');

            if (colon != NULL) {

                *colon = '\0';

                client_pid = (pid_t)atoi(temp + 4);

                /*
                 * Send SIGUSR1 to client
                 */
                kill(client_pid, SIGUSR1);
            }
        }
    }

    close(read_fd);
    close(write_fd);

    exit(EXIT_SUCCESS);
}

/* -------------------------------------------------- */
/* MAIN SERVER                                        */
/* -------------------------------------------------- */
int main()
{
    int i;

    /* Register signals */
    signal(SIGINT, handle_sigint);
    signal(SIGCHLD, handle_sigchld);
    signal(SIGUSR1, handle_sigusr1);

    printf("====================================\n");
    printf("      MULTI-CLIENT FIFO SERVER\n");
    printf("====================================\n");

    /*
     * Create six FIFOs:
     *
     * client1 -> server
     * server -> client1
     *
     * client2 -> server
     * server -> client2
     *
     * client3 -> server
     * server -> client3
     */
    create_fifos();

    printf("Server: FIFOs created.\n\n");

    /*
     * Create three handler processes
     */
    for (i = 0; i < NUM_CLIENTS; i++) {

        handler_pids[i] = fork();

        if (handler_pids[i] < 0) {

            perror("fork");
            remove_fifos();
            exit(EXIT_FAILURE);
        }

        if (handler_pids[i] == 0) {

            /*
             * Child becomes handler
             */
            client_handler(i);
        }
    }

    printf("Server: All handler processes created.\n");

    for (i = 0; i < NUM_CLIENTS; i++) {
        printf("Handler %d PID = %d\n",
               i + 1, handler_pids[i]);
    }

    printf("\nServer is running...\n");
    printf("Press Ctrl+C to stop the server.\n");

    /*
     * Server waits until SIGINT
     */
    while (server_running) {
        pause();
    }

    printf("\nServer shutting down...\n");

    /*
     * Terminate handler processes
     */
    for (i = 0; i < NUM_CLIENTS; i++) {

        if (handler_pids[i] > 0) {
            kill(handler_pids[i], SIGTERM);
        }
    }

    /*
     * Wait for all handler processes
     */
    for (i = 0; i < NUM_CLIENTS; i++) {
        waitpid(handler_pids[i], NULL, 0);
    }

    /*
     * Remove FIFOs
     */
    remove_fifos();

    printf("Server: FIFOs removed.\n");
    printf("Server: Terminated.\n");

    return 0;
}
