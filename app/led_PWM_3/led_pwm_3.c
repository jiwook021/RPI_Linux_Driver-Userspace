#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int count = 0; 
int duty = 0 ;
int count_duty = 0;
int main(int argc, char **argv) {

int fd;
char buf;

//if(argc <= 1) {
//        printf("Usage : ./gpioled_test on or off\n");
//        return -1;
//}
system("sudo mknod -m 666 /dev/gpioled3 c 201 0");
//system("sudo chmod 666 /dev/gpioled3");

fd = open("/dev/gpioled3", O_WRONLY);
if(fd == -1) {
        printf("Device Open ERROR!\n");
        exit(-1);
}

//if(argv[1][0] == 'o' && argv[1][1] == 'n')
//        buf = 1;
//else if(argv[1][0]=='o' && argv[1][1]=='f' && argv[1][2]=='f')
//        buf = 0;

buf =0; 
while(1)
{
        if(count == 100)
        {
                count = 0;
                buf = 1;
                write(fd, &buf,1);
        }
        
        if(count_duty == 100)
        {
                count_duty = 0; 
                duty ++;
                if(duty ==99) duty = 1;
                
        }
        
        else if(count == duty)
        {
                buf = 0;
                write(fd, &buf, 1);
        }
        count++;
        count_duty++;
        usleep(100);
}

write(fd, &buf, 1); // LED control
close(fd);

return 0;
}
