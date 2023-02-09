#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int LED_fd, inout_fd;
    char value = 1;


    LED_fd = open("/sys/kernel/poly_sysfs/haha_value", O_RDWR);
    inout_fd = open("/sys/kernel/poly_sysfs/PIN_IO_value", O_RDWR);
    
    int i = 0;
    char buffer[2];


    write(inout_fd, "180", 3);
    

    for(int i = 0;i<20;i++) {

        if (i == 10)
        {
            write(inout_fd, "270", 3);
        }
        
        write(LED_fd, "1", 1);
        printf("w = 1\n");
       
        lseek(LED_fd, 0, SEEK_SET);
        sleep(1);
        write(LED_fd, "0", 1);
        printf("w = 0\n");

        sleep(1);

    }

    close(LED_fd);
    close(inout_fd);
    return EXIT_SUCCESS;
}