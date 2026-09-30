#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define BUFFER_SIZE 256

volatile sig_atomic_t response_received = 0;

/* -------------------------------------------------- */
/* SIGUSR1 handler                                    */
/* -------------------------------------------------- */
void handle_sigusr1(int sig)
{
    (void)sig;
    response_received = 1;
}

/* -------------------------------------------------- */
/* MAIN                                               */
/* -------------------------------------------------- */
int main(int argc, char *argv[])
{
    int client_no;

    int write_fd;
    int read_fd;

    char client_fifo[50];
    char server_fifo[50];

    char message[BUFFER_SIZE];
    char full_message[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    struct sigaction sa;

    /*
     * Check command-line argument.
     */
    if (argc != 2) {

        printf("Usage: %s <client_number>\n", argv[0]);
        printf("Example: %s 1\n", argv[0]);

        return 1;
    }

    client_no = atoi(argv[1]);

    /*
     * Only clients 1, 2 and 3 are allowed.
     */
    if (client_no < 1 || client_no > 3) {

        printf("Invalid client number.\n");
        printf("Use 1, 2 or 3.\n");

        return 1;
    }

    /*
     * Create FIFO names.
     */
    snprintf(client_fifo,
             sizeof(client_fifo),
             "client%d_to_server",
             client_no);

    snprintf(server_fifo,
             sizeof(server_fifo),
             "server_to_client%d",
             client_no);

    /*
     * Install SIGUSR1 handler.
     */
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handle_sigusr1;

    sigemptyset(&sa.sa_mask);

    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    /*
     * Block SIGUSR1 before communication.
     *
     * This prevents the signal from being missed.
     */
    sigset_t block_mask;
    sigset_t old_mask;
    sigset_t wait_mask;

    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);

    if (sigprocmask(SIG_BLOCK, &block_mask, &old_mask) == -1) {
        perror("sigprocmask");
        return 1;
    }

    /*
     * Open Client -> Server FIFO.
     */
    write_fd = open(client_fifo, O_WRONLY);

    if (write_fd == -1) {
        perror("open client-to-server FIFO");
        return 1;
    }

    /*
     * Open Server -> Client FIFO.
     */
    read_fd = open(server_fifo, O_RDONLY);

    if (read_fd == -1) {
        perror("open server-to-client FIFO");

        close(write_fd);

        return 1;
    }

    printf("========================================\n");
    printf("              CLIENT %d\n", client_no);
    printf("========================================\n");

    printf("Client PID = %d\n", getpid());
    printf("Connected to server.\n");
    printf("Type a message and press Enter.\n");
    printf("Type 'exit' to terminate.\n\n");

    /*
     * Create wait mask.
     *
     * SIGUSR1 will be unblocked only while
     * sigsuspend() is waiting.
     */
    wait_mask = old_mask;
    sigdelset(&wait_mask, SIGUSR1);

    while (1) {

        printf("Client %d: ", client_no);
        fflush(stdout);

        /*
         * Read message from user.
         */
        if (fgets(message,
                  sizeof(message),
                  stdin) == NULL) {
            break;
        }

        /*
         * Remove newline.
         */
        message[strcspn(message, "\n")] = '\0';

        /*
         * Create:
         *
         * PID:<client_pid>:<message>
         *
         * Example:
         *
         * PID:12345:Hello Server
         */
        snprintf(full_message,
                 sizeof(full_message),
                 "PID:%d:%s",
                 getpid(),
                 message);

        /*
         * Reset flag before sending.
         */
        response_received = 0;

        /*
         * Send message to server.
         */
        if (write(write_fd,
                  full_message,
                  strlen(full_message) + 1) == -1) {

            perror("write");

            break;
        }

        /*
         * Wait for SIGUSR1 from server.
         *
         * Since SIGUSR1 is blocked before the write,
         * the signal cannot be lost.
         */
        while (!response_received) {
            sigsuspend(&wait_mask);
        }

        /*
         * Read response from server.
         */
        memset(response, 0, sizeof(response));

        ssize_t n = read(read_fd,
                         response,
                         sizeof(response) - 1);

        if (n > 0) {

            response[n] = '\0';

            printf("Server: %s\n\n", response);
        }

        /*
         * Check for exit.
         */
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }

    /*
     * Restore original signal mask.
     */
    sigprocmask(SIG_SETMASK, &old_mask, NULL);

    close(write_fd);
    close(read_fd);

    printf("Client %d terminated.\n", client_no);

    return 0;
}
