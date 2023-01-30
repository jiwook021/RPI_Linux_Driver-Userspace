#include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <poll.h>

 int main(int argc, char *argv[]) {
    int fd;
    char value;
    struct pollfd poll_gpio;

    poll_gpio.events = POLLPRI;

    // export GPIO
    fd = open ("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) {
       fprintf(stderr, "Failed to open 1\n");
       return(-1);
   }
    write (fd, "17", 4);
    close (fd);

    // configure as input
    fd = open ("/sys/class/gpio/gpio17/direction", O_WRONLY);
    if (-1 == fd) {
       fprintf(stderr, "Failed to open 2\n");
       return(-1);
   }
    write (fd, "in", 3);
    close (fd);

    // configure interrupt
    fd = open ("/sys/class/gpio/gpio17/edge", O_WRONLY);
     if (-1 == fd) {
       fprintf(stderr, "Failed to open  3\n");
       return(-1);
   }
    write (fd, "rising", 7); // configure as rising edge
    
    close (fd);

    // open value file
    fd = open("/sys/class/gpio/gpio17/value", O_RDONLY );
     if (-1 == fd) {
       fprintf(stderr, "Failed to open pio 4\n");
       return(-1);
   }
    poll_gpio.fd = fd;

    //remake main.

    poll (&poll_gpio, 1, -1); // discard first IRQ
    read (fd, &value, 1);

    int fd2;
   
    fd2 = open ("/sys/class/gpio/export", O_WRONLY);
    write (fd2, "18", 4);
    close (fd2);

    // configure as input
    fd2 = open ("/sys/class/gpio/gpio18/direction", O_WRONLY);
    write (fd2, "out", 3);
    close (fd2);
    fd2 = open("/sys/class/gpio/gpio18/value", O_WRONLY);
    // wait for interrupt
    int i = 0; 
    char buffer [2];    
    
    while (1) {
         poll (&poll_gpio, 1, -1);
         if ((poll_gpio.revents & POLLPRI) == POLLPRI) {
            i++;
            if(i%2 == 0)
            {
                write(fd2, "0", 2);
            } 
            else
            {
                write(fd2, "1", 2);
            } 
            lseek(fd, 0, SEEK_SET);
            read (fd, &value, 1);
            usleep  (10);
            printf("Interrupt GPIO val: %c\n", value);

         }
    }

    close(fd); //close value file
    return EXIT_SUCCESS;
 }