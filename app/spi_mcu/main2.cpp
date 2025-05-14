#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <cstring>

int main() {
    int fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0) {
        std::cerr << "Cannot open SPI device" << std::endl;
        return -1;
    }
    
    // Configure SPI
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 100000; // Try a slower speed (100 KHz)
    
    ioctl(fd, SPI_IOC_WR_MODE, &mode);
    ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    
    // Test pattern - simple bytes
    uint8_t tx_data[4] = {0xAA, 0x55, 0xFF, 0x00};
    uint8_t rx_data[4] = {0};
    
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_data,
        .rx_buf = (unsigned long)rx_data,
        .len = 4,
        .speed_hz = speed,
        .delay_usecs = 10, // Add a small delay
        .bits_per_word = bits,
    };
    
    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        std::cerr << "SPI transfer failed" << std::endl;
        close(fd);
        return -1;
    }
    
    // Print received data
    std::cout << "Sent: ";
    for (int i = 0; i < 4; i++) {
        std::cout << std::hex << (int)tx_data[i] << " ";
    }
    std::cout << "\nReceived: ";
    for (int i = 0; i < 4; i++) {
        std::cout << std::hex << (int)rx_data[i] << " ";
    }
    std::cout << std::endl;
    
    close(fd);
    return 0;
}