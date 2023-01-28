#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
//#define GPIO_BASE 0x3F200000
#define GPIO_BASE 0x00200000
// #define GPFSEL1 0x04
// #define GPSET0 0x1C
// #define GPCLR0 0x28

int fd;
char *gpio_memory_map;
volatile unsigned int* gpio;

void  INThandler(int);

int main()
{
    int fd = open( "/dev/gpiomem", O_RDWR|O_SYNC );
    if ( fd < 0 ){
        printf("can't open /dev/gpiomem \n");
        return -1;
    }
    else
    {
        printf("open /dev/gpiomem success \n");
    }

    char *gpio_memory_map = (char *)mmap( 0, 4096, PROT_READ|PROT_WRITE,
                                          MAP_SHARED, fd, GPIO_BASE );
    if ( gpio_memory_map == MAP_FAILED )
    {
        printf(" mmap Error \n");
        return -1;
    }
    else
    {
        printf("mmap Success \n");
    }

    volatile unsigned int* gpio = (volatile unsigned int*)gpio_memory_map;
    // volatile unsigned int* gpio_direction_GPFSEL1 = gpio + 0x04/4;
    // volatile unsigned int* gpio_set_GPSET0 = gpio + 0x1C/4;
    // volatile unsigned int* gpio_set_GPCLR0 = gpio + 0x28/4;

    *(gpio + 0x04/4) &= ~(0x03 << 24); //clear pin 18 to 00
    *((gpio + 0x04/4)) |= 0x01 << 24; //set for pin 18 to set output


    *(gpio + 0x04/4) &= ~(0x03 << 21); //clear for pin 17 to 00
    *((gpio + 0x04/4)) |= 0x00 << 21; //set for pin 17 to set input


    signal(SIGINT, INThandler);

    while(1)
    {

        int in = *(gpio + 0X34/4);
        if(in & (0x01 << 17)) //if high
        {
            *(gpio + 0x1C/4) |= (1<<18); //set
        }
        else
        {
            *(gpio + 0x28/4) |= (1<<18);   //clr    
        }
    }
    munmap( gpio_memory_map, 4096);
    close(fd);

    return 0;
}

void INThandler(int sig)
{
    printf("\ninterrupt by %d",sig);
    munmap( gpio_memory_map, 4096);
    close(fd);
    printf("\nclosed fd and memory unmammped. \nend the process by exit(0)\n");
    exit(0);
}

