//cat / proc / cpuinfo
//
//================================================
//
///proc / cpuinfo — information about the CPU(s) on the system
/// proc / meminfo
//
/// proc / modules
/// proc / devices — registered character and block major numbers
//
/// proc / iomem — on - system physical RAM and bus device addresses
/// proc / ioports — on - system I / O port addresses(especially for x86 systems)
//
/// proc / interrupts — registered interrupt request numbers
/// proc / softirqs — registered soft IRQs
//
/// proc / swaps — currently active swaps
/// proc / kallsyms — running kernel symbols, including from loaded modules
//
/// proc / partitions — currently connected block devices and their partitions
/// proc / filesystems — currently active filesystem drivers
//
//
//================================================


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {

    int fd;
    char value = 1;
    fd = open("/proc/haha/haha_proc", O_RDWR);
    int i = 0;
    char buffer[2];

    while(1) {
        write(fd, "1", 1);
        printf("w = 1\n");
        //lseek(fd, 0, SEEK_SET);
        sleep(1);
        write(fd, "0", 1);
        printf("w = 0\n");
        sleep(1);
    }
    close(fd); //close value file
    return EXIT_SUCCESS;
}