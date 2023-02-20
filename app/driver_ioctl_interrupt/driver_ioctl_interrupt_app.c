// sudo mknod /dev/gpioled c 201 0
//sudo chmod 777 /dev/gpioled

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>



int main(int argc, char** argv) {

    system("sudo mknod -m 666 /dev/driver-control c 207 0");
    
    int fd;
    char buf;

    if (argc <= 1) {
        printf("Usage : ./gpioled_test on or off\n");
        return -1;
    }

    fd = open("/dev/driver-control", O_WRONLY);
    if (fd == -1) {
        printf("Device Open ERROR!\n");
        exit(-1);
    }


    //ioctl(fd, 0,0x01);

    if (argv[1][0] == 'o' && argv[1][1] == 'n')
    {
        ioctl(fd, 1, 0x01);
    }
    else if (argv[1][0] == 'o' && argv[1][1] == 'f' && argv[1][2] == 'f')
    {
        ioctl(fd, 0, 0x01);
    }

    close(fd);

    return 0;
}
