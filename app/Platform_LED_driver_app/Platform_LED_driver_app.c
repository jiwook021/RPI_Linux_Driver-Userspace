// app_misc.c



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

// 생성된 장치파일 위치 및 이름
#define NODE_NAME "/dev/Platform_LED"

int main(int argc, char** argv) 
{

    int buf;

    if (argc <= 1) {
        printf("Usage : ./out on or off\n");
        return -1;
    }
    system("sudo chmod 666 /dev/Platform_LED");
    // 파일 열기
    // 설정한 open 함수가 불린다.
    int fd = open(NODE_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("%s open error... \n", NODE_NAME);
        return -1;
    }
    
    if (argv[1][0] == 'o' && argv[1][1] == 'n')
    {
        buf = 1;
        write(fd, &buf, 1); 
    }
    else if (argv[1][0] == 'o' && argv[1][1] == 'f' && argv[1][2] == 'f')
    {
        buf = 0;
        write(fd, &buf, 1);
    }


    // 설정한 release 함수가 불린다.
    close(fd);
}