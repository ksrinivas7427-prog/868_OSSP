#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main() {
 int pid;
 int fd[2];
 char msg[] = "hello child!";
 char buffer[20];
 
 if(pipe(fd) == -1){
    perror("fail to create a pipe\n");
    exit(0);
    }
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
