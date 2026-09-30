#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#define A2B "fifoA"
#define B2A "fifoB"
int main() {
    int fd;
    char msg[100];
mkfifo(A2B, 0666);
mkfifo(B2A, 0666);

int pid = fork();

if (pid < 0) {
    printf("failed to create child\n");
    exit(0);
}

else if (pid == 0) {
    // child process - reading

    fd = open(A2B, O_RDONLY);

    while (1) {

        read(fd, msg, 100);

        printf("User A: %s\n", msg);

        if (strcmp(msg, "bye") == 0)
            break;
    }

    close(fd);
}

else {
    // parent process - writing

    while (1) {

        printf("User B: ");

        fgets(msg, 100, stdin);

        msg[strcspn(msg, "\n")] = '\0';

        fd = open(B2A, O_WRONLY);

        write(fd, msg, strlen(msg) + 1);

        close(fd);

        if (strcmp(msg, "bye") == 0)
            break;
    }
}

return 0;
}
