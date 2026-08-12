#include <stdio.h>
#include <unistd.h>

int main()
{
    pid_t pid;

    pid = fork();

    if (pid == 0)
    {
        printf("Child Process\n");
        printf("PID = %d\n", getpid());
    }
    else if (pid > 0)
    {
        printf("Parent Process\n");
        printf("Child PID = %d\n", pid);
    }
    else
    {
        printf("Error in process creation\n");
    }

    return 0;
}
