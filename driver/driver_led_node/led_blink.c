#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char **argv) {

int fd;
char buf;

//if(argc <= 1) {
//        printf("Usage : ./gpioled_test on or off\n");
//        return -1;
//}

fd = open("/dev/gpioled2", O_WRONLY);
if(fd == -1) {
        printf("Device Open ERROR!\n");
        exit(-1);
}

//if(argv[1][0] == 'o' && argv[1][1] == 'n')
//        buf = 1;
//else if(argv[1][0]=='o' && argv[1][1]=='f' && argv[1][2]=='f')
//        buf = 0;

buf =0; 
int i =0;
for(i = 0; i<10; i++)
{
        buf = !buf;
        write(fd,&buf,1);
        usleep(1000);
}

write(fd, &buf, 1); // LED control
close(fd);

return 0;
}
