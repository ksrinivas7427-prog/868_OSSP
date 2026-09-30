#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t producer, consumer;

    // Create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create Producer Process
    producer = fork();

    if (producer == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (producer == 0) {
        // ---------------- Producer Process ----------------
        // Producer executes: ls -l

        // Close unused read end
        close(pipefd[0]);

        // Redirect stdout to pipe's write end
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        // Close original write-end descriptor
        close(pipefd[1]);

        // Replace process with ls -l
        execlp("ls", "ls", "-l", (char *)NULL);

        // execlp() returns only if there is an error
        perror("execlp ls");
        exit(EXIT_FAILURE);
    }

    // Create Consumer Process
    consumer = fork();

    if (consumer == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (consumer == 0) {
        // ---------------- Consumer Process ----------------
        // Consumer executes: grep ".c"

        // Close unused write end
        close(pipefd[1]);

        // Redirect stdin to pipe's read end
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        // Close original read-end descriptor
        close(pipefd[0]);

        // Replace process with grep ".c"
        execlp("grep", "grep", ".c", (char *)NULL);

        // execlp() returns only if there is an error
        perror("execlp grep");
        exit(EXIT_FAILURE);
    }

    // ---------------- Parent Process ----------------

    // Parent does not need either end of the pipe
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for Producer
    if (waitpid(producer, NULL, 0) == -1) {
        perror("waitpid producer");
        exit(EXIT_FAILURE);
    }

    // Wait for Consumer
    if (waitpid(consumer, NULL, 0) == -1) {
        perror("waitpid consumer");
        exit(EXIT_FAILURE);
    }

    printf("Parent: Both processes have terminated.\n");

    return 0;
}
