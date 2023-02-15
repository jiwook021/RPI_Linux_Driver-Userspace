// sudo mknod /dev/gpioled c 201 0
//sudo chmod 777 /dev/gpioled

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#define  LED_DEV_MAGIC   'Y'
 
#define  LED_INIT    _IO(LED_DEV_MAGIC,   0)
#define  LED_ON      _IOW(LED_DEV_MAGIC, 1, unsigned char)
#define  LED_OFF     _IOW(LED_DEV_MAGIC, 2, unsigned char) 



int main(int argc, char **argv) {

    system("sudo mknod -m 666 /dev/gpioled_ioctl c 207 0");
    //system("sudo chmod 666 /dev/gpioled_ioctl");

int fd;
char buf;

if(argc <= 1) {
        printf("Usage : ./gpioled_test on or off\n");
        return -1;
}

fd = open("/dev/gpioled_ioctl", O_WRONLY);
if(fd == -1) {
        printf("Device Open ERROR!\n");
        exit(-1);
}

    
ioctl(fd, LED_INIT);  

if(argv[1][0] == 'o' && argv[1][1] == 'n')
{
    buf = 1;
    ioctl(fd, LED_ON, 0x01); 
}
else if(argv[1][0]=='o' && argv[1][1]=='f' && argv[1][2]=='f')
{
        buf = 0;
        ioctl(fd, LED_OFF, 0x00); 
}

//write(fd, &buf, 1); // LED control
close(fd);

return 0;
}
