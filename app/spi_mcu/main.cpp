#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <csignal>
#include <iomanip>

// Global flag for handling Ctrl+C
volatile sig_atomic_t running = true;

// Signal handler for Ctrl+C
void signalHandler(int signum) {
    std::cout << "\nInterrupt received, stopping application..." << std::endl;
    running = false;
}

class SPIController {
public:
    // Command definitions
    static const uint8_t CMD_LED_ON = 0x01;
    static const uint8_t CMD_LED_OFF = 0x02;
    static const uint8_t CMD_QUERY_STATE = 0x03;
    static const uint8_t CMD_READ_ANALOG = 0x04;  // New command for analog reading
    static const uint8_t CMD_GET_RESPONSE = 0xFF;
    
    // Response codes
    static const uint8_t RESP_PROCESSING = 0xAA;
    static const uint8_t RESP_ACK = 0x00;
    static const uint8_t RESP_LED_ON = 0x01;
    static const uint8_t RESP_LED_OFF = 0x02;
    static const uint8_t RESP_ERROR = 0xFF;
    
    /**
     * @brief Constructor - Initialize SPI communication with STM32
     * @param device SPI device path
     * @param speed SPI clock speed in Hz
     */
    SPIController(const std::string& device = "/dev/spidev0.0", uint32_t speed = 100000) {
        // Open SPI device
        fd = open(device.c_str(), O_RDWR);
        if (fd < 0) {
            throw std::runtime_error("Cannot open SPI device: " + device);
        }
        
        // Configure SPI settings
        uint8_t mode = SPI_MODE_0;  // CPOL=0, CPHA=0
        uint8_t bits = 8;
        // Set SPI mode
        if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
            close(fd);
            throw std::runtime_error("Cannot set SPI mode");
        }
        
        // Set bits per word
        if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
            close(fd);
            throw std::runtime_error("Cannot set bits per word");
        }
        
        // Set max speed
        if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
            close(fd);
            throw std::runtime_error("Cannot set SPI speed");
        }
        
        std::cout << "SPI connection established on " << device << std::endl;
    }
    
    /**
     * @brief Destructor - Close SPI connection
     */
    ~SPIController() {
        if (fd >= 0) {
            close(fd);
            std::cout << "SPI connection closed" << std::endl;
        }
    }
    
    /**
     * @brief Turn the LED ON
     * @return true if command successful, false otherwise
     */
    bool turnLedOn() {
        uint8_t response = sendCommand(CMD_LED_ON);
        return response == RESP_ACK;
    }
    
    /**
     * @brief Turn the LED OFF
     * @return true if command successful, false otherwise
     */
    bool turnLedOff() {
        uint8_t response = sendCommand(CMD_LED_OFF);
        return response == RESP_ACK;
    }
    
    /**
     * @brief Query the current LED state
     * @return string representation of the LED state
     */
    std::string queryLedState() {
        uint8_t response = sendCommand(CMD_QUERY_STATE);
        
        if (response == RESP_LED_ON) {
            return "ON";
        } else if (response == RESP_LED_OFF) {
            return "OFF";
        } else {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "UNKNOWN (Code: 0x%02X)", response);
            return std::string(buffer);
        }
    }
    
    /**
     * @brief Read analog value from PF10 (ADC3)
     * @return ADC value (0-255)
     */
    uint8_t readAnalogValue() {
        return sendCommand(CMD_READ_ANALOG);
    }
    
private:
    int fd = -1;  // SPI file descriptor
    
    /**
     * @brief Calculate simple XOR checksum
     * @param data Byte to calculate checksum for
     * @return Calculated checksum
     */
    uint8_t calculateChecksum(uint8_t data) const {
        return data ^ 0xFF;
    }
    
    /**
     * @brief Perform SPI transfer
     * @param tx_data Data to transmit
     * @param rx_data Buffer to store received data
     * @param length Number of bytes to transfer
     * @return 0 on success, negative on error
     */
    int spiTransfer(const uint8_t* tx_data, uint8_t* rx_data, size_t length) const {
        struct spi_ioc_transfer tr;
        memset(&tr, 0, sizeof(tr));
        
        tr.tx_buf = (unsigned long)tx_data;
        tr.rx_buf = (unsigned long)rx_data;
        tr.len = length;
        tr.speed_hz = 0;  // Use default speed configured in constructor
        tr.delay_usecs = 0;
        tr.bits_per_word = 0;  // Use default bits configured in constructor
        tr.cs_change = 0;
        
        return ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    }
    
    /**
     * @brief Send a command to STM32 and get response
     * @param command Command byte to send
     * @return Response code from STM32
     */
    /**
 * @brief Send a command to STM32 and get response
 * @param command Command byte to send
 * @return Response code from STM32
 */
uint8_t sendCommand(uint8_t command) {
    // Prepare command and checksum
    uint8_t tx_buffer[2] = {command, calculateChecksum(command)};
    uint8_t rx_buffer[2] = {0};
    
    std::cout << "Sending command: 0x" << std::hex << static_cast<int>(command) 
              << ", checksum: 0x" << static_cast<int>(calculateChecksum(command)) 
              << std::dec << std::endl;
    
    // First transfer - send command
    if (spiTransfer(tx_buffer, rx_buffer, 2) < 0) {
        std::cerr << "Error during SPI command transfer" << std::endl;
        return RESP_ERROR;
    }
    
    std::cout << "Initial response: [0x" << std::hex << static_cast<int>(rx_buffer[0]) 
              << ", 0x" << static_cast<int>(rx_buffer[1]) << "]" << std::dec << std::endl;
    
    // Allow more time for STM32 to process (increased)
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Fix: Send dummy bytes to synchronize the SPI communication
    tx_buffer[0] = 0x00;  // Dummy byte
    tx_buffer[1] = 0x00;  // Dummy byte
    
    if (spiTransfer(tx_buffer, rx_buffer, 2) < 0) {
        std::cerr << "Error during SPI synchronization" << std::endl;
        return RESP_ERROR;
    }
    
    std::cout << "Sync response: [0x" << std::hex << static_cast<int>(rx_buffer[0]) 
              << ", 0x" << static_cast<int>(rx_buffer[1]) << "]" << std::dec << std::endl;
    
    // Additional delay after synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Second transfer - request actual response
    tx_buffer[0] = CMD_GET_RESPONSE;
    tx_buffer[1] = calculateChecksum(CMD_GET_RESPONSE);
    
    std::cout << "Requesting response with command: 0x" << std::hex 
              << static_cast<int>(CMD_GET_RESPONSE) << std::dec << std::endl;
    
    if (spiTransfer(tx_buffer, rx_buffer, 2) < 0) {
        std::cerr << "Error during SPI response request" << std::endl;
        return RESP_ERROR;
    }
    
    std::cout << "Final response: [0x" << std::hex << static_cast<int>(rx_buffer[0]) 
              << ", 0x" << static_cast<int>(rx_buffer[1]) << "]" << std::dec << std::endl;
    
    // Verify checksum
    if (calculateChecksum(rx_buffer[0]) != rx_buffer[1]) {
        std::cerr << "Warning: Invalid checksum in response. Expected: 0x" 
                  << std::hex << static_cast<int>(calculateChecksum(rx_buffer[0]))
                  << ", Got: 0x" << static_cast<int>(rx_buffer[1]) << std::dec << std::endl;
        
        // Try up to 3 more times with increasing delays
        for (int retry = 1; retry <= 3; retry++) {
            std::cerr << "Retry #" << retry << " after " << (100 * retry) << "ms..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * retry));
            
            if (spiTransfer(tx_buffer, rx_buffer, 2) < 0) {
                std::cerr << "Error during SPI retry" << std::endl;
                continue;
            }
            
            std::cout << "Retry response: [0x" << std::hex << static_cast<int>(rx_buffer[0]) 
                      << ", 0x" << static_cast<int>(rx_buffer[1]) << "]" << std::dec << std::endl;
            
            if (calculateChecksum(rx_buffer[0]) == rx_buffer[1]) {
                std::cout << "Valid checksum on retry #" << retry << std::endl;
                break;
            }
        }
    }
    
    // Add a longer delay between commands for stability
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    return rx_buffer[0];  // Return the response value (ADC value or status)
}
    
    // Prevent copying
    SPIController(const SPIController&) = delete;
    SPIController& operator=(const SPIController&) = delete;
};

int main() {
    // Set up signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);
    
    try {
        SPIController spi_controller;
        unsigned int loopCount = 0;
        
        std::cout << "SPI Controller Started" << std::endl;
        std::cout << "Press Ctrl+C to exit" << std::endl;
        
        // Main operation loop
        while (running) {
            try {
                loopCount++;
                
                // Turn LED ON
                std::cout << "\n[" << loopCount << "] Turning LED ON..." << std::endl;
                if (spi_controller.turnLedOn()) {
                    std::cout << "Command successful" << std::endl;
                } else {
                    std::cout << "Command failed" << std::endl;
                }
                
                // Read analog value
                uint8_t analogValue = spi_controller.readAnalogValue();
                float voltage = (analogValue / 255.0f) * 3.3f;  // Convert to voltage (assuming 3.3V reference)
                
                std::cout << "Analog reading: " << static_cast<int>(analogValue) 
                          << " (approximately " << std::fixed << std::setprecision(2) 
                          << voltage << "V)" << std::endl;
                
                // Delay while LED is ON
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                // Check if we should stop
                if (!running) break;
                
                // Turn LED OFF
                std::cout << "\n[" << loopCount << "] Turning LED OFF..." << std::endl;
                if (spi_controller.turnLedOff()) {
                    std::cout << "Command successful" << std::endl;
                } else {
                    std::cout << "Command failed" << std::endl;
                }
                
                // Read analog value again while LED is off
                analogValue = spi_controller.readAnalogValue();
                voltage = (analogValue / 255.0f) * 3.3f;  // Convert to voltage (assuming 3.3V reference)
                
                std::cout << "Analog reading: " << static_cast<int>(analogValue) 
                          << " (approximately " << std::fixed << std::setprecision(2) 
                          << voltage << "V)" << std::endl;
                
                // Delay while LED is OFF
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                // Check if we should stop
                if (!running) break;
            } catch (const std::exception& e) {
                std::cerr << "Error during SPI communication: " << e.what() << std::endl;
                std::cerr << "Retrying in 2 seconds..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                
                if (!running) break;
            }
        }
        
        // Ensure the LED is turned off before exiting
        std::cout << "Turning LED OFF before exit..." << std::endl;
        spi_controller.turnLedOff();
        
        std::cout << "Application stopped after " << loopCount << " cycles" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }
}