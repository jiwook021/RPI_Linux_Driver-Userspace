//====================================

// app_misc.c



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

// 생성된 장치파일 위치 및 이름
#define NODE_NAME "/dev/drv_hello"

int main(int argc, char* argv[])
{
    system("sudo chmod 666 /dev/drv_hello");
    // 파일 열기
    // 설정한 open 함수가 불린다.
    int fd = open(NODE_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("%s open error... \n", NODE_NAME);
        return -1;
    }
    char buf;
    buf = 0;
    for (int i = 0; i < 10; i++)
    {
        buf = 1;
        write(fd, &buf, 1);
        sleep(1);
        buf = 0;
        write(fd, &buf, 1);
        sleep(1);
    }
    
    close(fd);
}

//probe -> led 5 times
//write -> gpioset
//