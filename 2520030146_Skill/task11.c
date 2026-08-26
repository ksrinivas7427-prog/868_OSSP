#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    printf("Web Browser is running. PID: %d\n", getpid());

    pid = fork();

    if (pid < 0) {
        printf("Fork failed!\n");
    }
    else if (pid == 0) {
        printf("Child process opening PDF. PID: %d\n", getpid());

        execlp("xdg-open", "xdg-open", "lecture_notes.pdf", NULL);

        printf("Failed to open PDF.\n");
    }
    else {
        printf("Parent browser is still running. PID: %d\n", getpid());

        wait(NULL);

        printf("PDF process completed.\n");
    }

    return 0;
}
