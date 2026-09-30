#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
int main() {
    int fd1, fd2;
    char msg[100];
// Create the FIFOs if they don't already exist
mkfifo("fifo1", 0666);
mkfifo("fifo2", 0666);

while (1) {

    // Open fifo1 for reading
    fd1 = open("fifo1", O_RDONLY);

    // Read message from User A
    read(fd1, msg, sizeof(msg));

    close(fd1);

    printf("User A: %s\n", msg);

    // Check for exit message
    if (strcmp(msg, "bye") == 0)
        break;

    printf("User B: ");
    fgets(msg, 100, stdin);

    // Remove newline character
    msg[strcspn(msg, "\n")] = '\0';

    // Open fifo2 for writing
    fd2 = open("fifo2", O_WRONLY);

    // Write message to User A
    write(fd2, msg, strlen(msg) + 1);

    close(fd2);

    // Check for exit message
    if (strcmp(msg, "bye") == 0)
        break;
}

return 0;
}
