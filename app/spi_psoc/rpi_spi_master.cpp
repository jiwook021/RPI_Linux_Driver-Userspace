// File: rpi_echo_loop.c
// Build: gcc -std=c11 -O2 -o rpi_echo_loop rpi_echo_loop.c
// Run:   sudo ./rpi_echo_loop

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

static volatile int keep_running = 1;
static void handle_sigint(int _) { keep_running = 0; }

int main(void) {
    const char *device = "/dev/spidev0.1";  // CE1 on BCM7
    int fd = open(device, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    // SPI 설정: 모드0, 8비트, 25kHz
    uint8_t mode = SPI_MODE_0;
    ioctl(fd, SPI_IOC_WR_MODE, &mode);
    uint8_t bits = 8;
    ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    uint32_t speed = 25000;
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    // 보낼 명령 배열
    const uint8_t cmds[] = { 0xA0, 0xA1, 0xA2, 0xA3 };
    const size_t n_cmds = sizeof(cmds)/sizeof(*cmds);

    signal(SIGINT, handle_sigint);
    printf("Press Ctrl+C to stop\n");

    while (keep_running) {
        for (size_t i = 0; i < n_cmds && keep_running; ++i) {
            uint8_t cmd = cmds[i];
            uint8_t rx;

            // Phase 1: send command byte
            struct spi_ioc_transfer tr = {
                .tx_buf        = (unsigned long)&cmd,
                .rx_buf        = (unsigned long)&rx,
                .len           = 1,
                .speed_hz      = speed,
                .bits_per_word = bits,
            };
            if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
                perror("SPI write failed");
                close(fd);
                return 1;
            }

            // 슬레이브 처리 대기
            usleep(100000);  // 100ms

            // Phase 2: 읽기
            uint8_t dummy = 0x00;
            tr.tx_buf = (unsigned long)&dummy;
            tr.rx_buf = (unsigned long)&rx;
            if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
                perror("SPI read failed");
                close(fd);
                return 1;
            }

            // 결과 출력
            printf("Cmd 0x%02X → Echo 0x%02X : %s\n",
                   cmd, rx, (rx == cmd) ? "OK" : "FAIL");

            // 다음 명령 전 짧은 대기
            usleep(200000);  // 200ms
        }
    }

    printf("Exiting...\n");
    close(fd);
    return 0;
}
