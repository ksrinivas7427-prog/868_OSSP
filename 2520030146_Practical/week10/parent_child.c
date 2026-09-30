#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int count = 0;
int main() {
 int pid;
 pid = fork();
 if (pid < 0) {
  printf("Problem is creating child\n");
  exit(0);
 }
 else if (pid == 0) {
  printf("Child process is executing\n");
  for (int i = 0; i <5; i++) {
   count--;
   printf("child count: %d\n", count);
  }  
 }
 else {
  printf("Parent process is executing\n");
  for (int i = 0; i < 5; i++) {
   count--;
   printf("parent count: %d\n", count);
  } 
  wait(NULL);
  printf("\n Final count: %d\n", count);
 }
 return 0;
}
