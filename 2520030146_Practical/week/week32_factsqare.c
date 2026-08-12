#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    int pid, num;

    printf("Enter num\n");
    scanf("%d", &num);

    pid = fork();

    if (pid < 0) {
        printf("Failed to create new process\n");
        exit(0);
    }

    else if (pid == 0) {
        // Child process - factorial
        unsigned int fact = 1;
        int temp = num;

        while (temp > 0) {
            fact = fact * temp;
            temp--;
        }

        printf("Factorial of %d is %u\n", num, fact);
    }

    else {
        // Parent process - square
        int sqr;

        wait(NULL);

        sqr = num * num;

        printf("Square of %d is %d\n", num, sqr);
    }

    return 0;
}
