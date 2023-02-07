#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {

    int fd;
    char value = 1;
    fd = open("/proc/haha/haha_proc", O_RDWR);
    int i = 0;
    char buffer[2];


    while(1) {
        
        write(fd, "1", 1);
        printf("w = 1\n");

        //lseek(fd, 0, SEEK_SET);
        sleep(1);
        write(fd, "0", 1);
        printf("w = 0\n");
        sleep(1);
    }
    close(fd); //close value file
    return EXIT_SUCCESS;
}