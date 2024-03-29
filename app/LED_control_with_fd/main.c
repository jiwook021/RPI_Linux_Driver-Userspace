#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1


#define POUT 7

static int GPIOExport(int pin)
{
#define BUFFER_MAX 3
   char buffer[BUFFER_MAX];
   ssize_t bytes_written;
   int fd;

   fd = open("/sys/class/gpio/export", O_WRONLY);
   if (-1 == fd) {
       fprintf(stderr, "Failed to open export for writing!\n");
       return(-1);
   }

   bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
   write(fd, buffer, bytes_written);
   close(fd);
   return(0);
}

static int
GPIOUnexport(int pin)
{
   char buffer[BUFFER_MAX];
   ssize_t bytes_written;
   int fd;

   fd = open("/sys/class/gpio/unexport", O_WRONLY);
   if (-1 == fd) {
       fprintf(stderr, "Failed to open unexport for writing!\n");
       return(-1);
   }

   bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
   write(fd, buffer, bytes_written);
   close(fd);
   return(0);
}

static int GPIODirection(int pin, int dir)
{
   static const char s_directions_str[]  = "in\0out";

#define DIRECTION_MAX 35
   char path[DIRECTION_MAX];
   int fd;

   snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
   fd = open(path, O_WRONLY);
   if (-1 == fd) {
       fprintf(stderr, "Failed to open gpio direction for writing!\n");
       return(-1);
   }

   if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
       fprintf(stderr, "Failed to set direction!\n");
       return(-1);
   }

   close(fd);
   return(0);
}

static int GPIORead(int pin)
{
#define VALUE_MAX 30
   char path[VALUE_MAX];
   char value_str[3];
   int fd;

   snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
   fd = open(path, O_RDONLY);
   if (-1 == fd) {
       fprintf(stderr, "Failed to open gpio value for reading!\n");
       return(-1);
   }

   if (-1 == read(fd, value_str, 3)) {
       fprintf(stderr, "Failed to read value!\n");
       return(-1);
   }

   close(fd);

   return(atoi(value_str));
}

static int GPIOWrite(int pin, int value)
{
   static const char s_values_str[] = "01";

   char path[VALUE_MAX];
   int fd;

   snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
   fd = open(path, O_WRONLY);
   if (-1 == fd) {
       fprintf(stderr, "Failed to open gpio value for writing!\n");
       return(-1);
   }

   if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
       fprintf(stderr, "Failed to write value!\n");
       return(-1);
   }

   close(fd);
   return(0);
}

/*
cd /sys/class/gpio/

echo 18 > export
echo out > direction
echo 1 > value
echo 0 > value

pin number> export
out > direction 
main on,off 5 times code 1seconds

*/


// int main()
// {
//     static const int PIN = 18;

//     int ret =0;
//     ret = GPIOExport(PIN);
//     ret = GPIODirection(PIN, 1);

//     for(int i =0; i<5; i++)
//     {
//         GPIOWrite(PIN, 0);
//         sleep(1);
//         GPIOWrite(PIN, 1);
//         sleep(1);
//     }

//     return 0;
// }


int main()
{
    static const int PIN = 18;


    int fd;
    char value;
    
    // export GPIO
    fd = open ("/sys/class/gpio/export", O_WRONLY);
    write (fd, "18", 4);
    close (fd);

    // configure as input
    fd = open ("/sys/class/gpio/gpio18/direction", O_WRONLY);
    write (fd, "out", 3);
    close (fd);


    fd = open("/sys/class/gpio/gpio18/value", O_WRONLY);
    for(int i =0; i<5; i++)
    {
        write (fd, "1", 2);
        sleep(1);
        write (fd, "0", 2);
        sleep(1);
    }   

    close (fd);
    return 0;
}