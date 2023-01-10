#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

//#define GPIO_BASE 0x3F200000
#define GPIO_BASE 0x00200000
#define GPFSEL1 0x04
#define GPSET0 0x1C
#define GPCLR0 0x28

int fd;
char *gpio_memory_map;
volatile unsigned int* gpio;


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
    volatile unsigned int* gpio_direction_GPFSEL1 = gpio + GPFSEL1/4;
    volatile unsigned int* gpio_set_GPSET0 = gpio + GPSET0/4;
    volatile unsigned int* gpio_set_GPCLR0 = gpio + GPCLR0/4;

    *gpio_direction_GPFSEL1 &= ~(0x03 << 24); 
    *gpio_direction_GPFSEL1 |= 0x01 << 24; 

    int i;

    for ( i=0; i<5; i++ )
    {
        *gpio_set_GPSET0 |= (1<<18);
        printf("gpio18(LED) On \n");
        usleep (1000000);

        *gpio_set_GPCLR0 |= (1<<18);
        printf("gpio18(LED) Off \n");
        usleep (1000000);
    }

    munmap( gpio_memory_map, 4096);
    close(fd);

    return 0;
}