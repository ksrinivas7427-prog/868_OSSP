#include <stdio.h>

int main() {
 int pid;
 pid = fork();
 if (pid < 0) {
    printf("Failure in process creation\n");
    exit(0);
 }
 else if (pid == 0) {
    printf("I am in Child Process\n");
 }
 else {
    printf("I am in Parent Process\n");
 }
 return 0;
}
