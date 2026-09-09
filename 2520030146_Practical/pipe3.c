#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int pid;
    int p1[2], p2[2];

    char msg1[] = "Hello Child!";
    char msg2[] = "Hello Parent!";
    char buffer[50];

    pipe(p1);
    pipe(p2);

    pid = fork();

    if (pid < 0) {
        printf("Process creation failed\n");
        exit(1);
    }

    else if (pid == 0) {

        printf("I am Child Process\n");

        close(p1[1]);
        read(p1[0], buffer, sizeof(buffer));
        printf("Message from Parent: %s\n", buffer);
        close(p1[0]);

        close(p2[0]);
        write(p2[1], msg2, strlen(msg2) + 1);
        close(p2[1]);
    }
    else {

        printf("I am Parent Process\n");

        close(p1[0]);
        write(p1[1], msg1, strlen(msg1) + 1);
        close(p1[1]);

        close(p2[1]);
        read(p2[0], buffer, sizeof(buffer));
        printf("Message from Child: %s\n", buffer);
        close(p2[0]);
    }

    return 0;
}
