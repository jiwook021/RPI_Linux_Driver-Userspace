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
    write (fd, "17", 4);
    close (fd);

    // configure as input
    fd = open ("/sys/class/gpio/gpio17/direction", O_WRONLY);
    write (fd, "in", 3);
    close (fd);

    // configure interrupt
    fd = open ("/sys/class/gpio/gpio17/edge", O_WRONLY);
    write (fd, "rising", 7); // configure as rising edge
    close (fd);

    // open value file
    fd = open("/sys/class/gpio/gpio17/value", O_RDONLY );
    poll_gpio.fd = fd;

    //remake main.

    poll (&poll_gpio, 1, -1); // discard first IRQ
    read (fd, &value, 1);

    // wait for interrupt
    while (1) {
         poll (&poll_gpio, 1, -1);
         if ((poll_gpio.revents & POLLPRI) == POLLPRI) {
             lseek(fd, 0, SEEK_SET);
             read (fd, &value, 1);
             usleep  (50);
             printf("Interrupt GPIO val: %c\n", value);
         }
    }

    close(fd); //close value file
    return EXIT_SUCCESS;
 }