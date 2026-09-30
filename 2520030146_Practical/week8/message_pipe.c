#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
int main() {
    int fd1, fd2;
    char msg[100];
// Create two named pipes
mkfifo("fifo1", 0666);
mkfifo("fifo2", 0666);

while (1) {
    printf("User A: ");
    fgets(msg, 100, stdin);

    // Remove newline character
    msg[strcspn(msg, "\n")] = '\0';

    // Open fifo1 for writing
    fd1 = open("fifo1", O_WRONLY);

    // Write message to fifo1
    write(fd1, msg, strlen(msg) + 1);

    close(fd1);

    // Check for exit message
    if (strcmp(msg, "bye") == 0)
        break;

    // Open fifo2 for reading
    fd2 = open("fifo2", O_RDONLY);

    // Read message from fifo2
    read(fd2, msg, sizeof(msg));

    close(fd2);

    printf("User B: %s\n", msg);

    // Check for exit message
    if (strcmp(msg, "bye") == 0)
        break;
}

return 0;
}
