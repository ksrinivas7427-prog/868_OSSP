#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


int main() {
    char bfr[100];
    printf("receiving message from writer\n");
    int fd = open("/tmp/navadeep_fifo", O_RDONLY);
    read(fd, bfr, sizeof(bfr));
    printf("Received message: %s\n", bfr);
    close(fd);
    return 0;
}
