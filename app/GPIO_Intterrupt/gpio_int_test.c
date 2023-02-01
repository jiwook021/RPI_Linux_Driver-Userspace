// gpio_int_test.c



//sudo mknod /dev/gpiobtn c 202 0
//sudo chmod 777 /dev/gpiobtn

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
int main(void) {
int fd;

char buf[40];
fd = open("/dev/gpiobtn", O_RDWR);
    
if(fd < 0) {
    printf( "Device Open ERROR!\n");
    exit(1);
}
    
printf("Please push the GPIO button!\n");
    
while(1) {
    read(fd, buf, 40); // read
    if(buf[0] != '\0')
        printf("%s\n", buf);
}
    
close(fd);
    
return 0;
}