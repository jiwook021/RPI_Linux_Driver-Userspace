#include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <unistd.h>

 int main(int argc, char *argv[]) {
   
    int fd;
    char value =1;    
    fd = open("/sys/kernel/haha_sysfs/haha_value", O_RDWR);
    int i = 0; 
    char buffer [2];    
    
    while (1) {
        if(value == '0')
        {
            write(fd, "0", 1);
            printf("w = 0\n");
        } 
        else
        {
            write(fd, "1", 1);
            printf(" w = 1\n");
        } 
        lseek(fd, 0, SEEK_SET);
             
        if( -1 == read (fd, &value, 1))
        {
            printf("wrong\n");
        }
        usleep  (100);
    }
    close(fd); //close value file
    return EXIT_SUCCESS;
 }