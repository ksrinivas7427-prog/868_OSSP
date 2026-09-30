#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define NUM_CLIENTS 3
#define BUFFER_SIZE 256
#define RESPONSE_SIZE 512

/* FIFO names */
const char *client_to_server[NUM_CLIENTS] = {
    "client1_to_server",
    "client2_to_server",
    "client3_to_server"
};

const char *server_to_client[NUM_CLIENTS] = {
    "server_to_client1",
    "server_to_client2",
    "server_to_client3"
};

pid_t handler_pids[NUM_CLIENTS];

volatile sig_atomic_t server_running = 1;

void handle_sigint(int sig)
{
    (void)sig;
    server_running = 0;
}

void handle_sigusr1(int sig)
{
    (void)sig;
    printf("Server: SIGUSR1 received.\n");
}

void handle_sigchld(int sig)
{
    (void)sig;
}

void create_fifos(void)
{
    int i;

    for (i = 0; i < NUM_CLIENTS; i++) {

        if (mkfifo(client_to_server[i], 0666) == -1) {
            if (errno != EEXIST) {
                perror("mkfifo client_to_server");
                exit(EXIT_FAILURE);
            }
        }

        if (mkfifo(server_to_client[i], 0666) == -1) {
            if (errno != EEXIST) {
                perror("mkfifo server_to_client");
                exit(EXIT_FAILURE);
            }
        }
    }
}


void remove_fifos(void)
{
    int i;

    for (i = 0; i < NUM_CLIENTS; i++) {
        unlink(client_to_server[i]);
        unlink(server_to_client[i]);
    }
}


void client_handler(int client_no)
{
    int read_fd;
    int write_fd;

    char buffer[BUFFER_SIZE];
    char response[RESPONSE_SIZE];

    char *client_fifo = (char *)client_to_server[client_no];
    char *server_fifo = (char *)server_to_client[client_no];

    printf("Handler %d started. PID = %d\n",
           client_no + 1, getpid());

    /*
     * Open both FIFOs.
     * O_RDWR prevents blocking while opening the FIFO.
     */
    read_fd = open(client_fifo, O_RDWR);

    if (read_fd == -1) {
        perror("open client FIFO");
        exit(EXIT_FAILURE);
    }

    write_fd = open(server_fifo, O_RDWR);

    if (write_fd == -1) {
        perror("open server FIFO");
        close(read_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {

        memset(buffer, 0, sizeof(buffer));

     
        ssize_t n = read(read_fd, buffer, sizeof(buffer) - 1);

        if (n <= 0) {
            continue;
        }

        buffer[n] = '\0';

        printf("Handler %d received: %s\n",
               client_no + 1, buffer);

        if (strncmp(buffer, "PID:", 4) == 0) {

            char temp[BUFFER_SIZE];
            char *colon;
            pid_t client_pid;

            strncpy(temp, buffer, sizeof(temp) - 1);
            temp[sizeof(temp) - 1] = '\0';

        
            colon = strchr(temp + 4, ':');

            if (colon != NULL) {

                *colon = '\0';

                client_pid = (pid_t)atoi(temp + 4);

             
                if (strcmp(colon + 1, "exit") == 0) {

                    const char response_exit[] =
                        "Server: Client disconnected.";

                    write(write_fd,
                          response_exit,
                          strlen(response_exit) + 1);

                    kill(client_pid, SIGUSR1);

                    printf("Handler %d terminating.\n",
                           client_no + 1);

                    break;
                }

             
                snprintf(response,
                         sizeof(response),
                         "Server Handler %d: Message received -> %.400s",
                         client_no + 1,
                         colon + 1);

            
                write(write_fd,
                      response,
                      strlen(response) + 1);

                kill(client_pid, SIGUSR1);
            }
        }
    }

    close(read_fd);
    close(write_fd);

    exit(EXIT_SUCCESS);
}

int main(void)
{
    int i;

  
    signal(SIGINT, handle_sigint);
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGCHLD, handle_sigchld);

    printf("========================================\n");
    printf("       MULTI-CLIENT FIFO SERVER\n");
    printf("========================================\n");

  
    create_fifos();

    printf("Server: FIFOs created successfully.\n\n");

   
    for (i = 0; i < NUM_CLIENTS; i++) {

        handler_pids[i] = fork();

        if (handler_pids[i] == -1) {
            perror("fork");

            for (int j = 0; j < i; j++) {
                kill(handler_pids[j], SIGTERM);
            }

            remove_fifos();
            exit(EXIT_FAILURE);
        }

        if (handler_pids[i] == 0) {

          
            client_handler(i);
        }
    }

    printf("Server: All handler processes created.\n");

    for (i = 0; i < NUM_CLIENTS; i++) {
        printf("Handler %d PID = %d\n",
               i + 1,
               handler_pids[i]);
    }

    printf("\nServer PID = %d\n", getpid());
    printf("Server is running...\n");
    printf("Press Ctrl+C to stop the server.\n\n");

   
    while (server_running) {
        pause();
    }

    printf("\nServer: Shutting down...\n");

    for (i = 0; i < NUM_CLIENTS; i++) {

        if (handler_pids[i] > 0) {
            kill(handler_pids[i], SIGTERM);
        }
    }

 
    for (i = 0; i < NUM_CLIENTS; i++) {

        if (waitpid(handler_pids[i], NULL, 0) == -1) {

            if (errno != ECHILD) {
                perror("waitpid");
            }
        }
    }


    remove_fifos();

    printf("Server: FIFOs removed.\n");
    printf("Server: Terminated successfully.\n");

    return 0;
}
