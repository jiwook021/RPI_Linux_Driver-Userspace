# Step-by-Step Explanation: gpio_interrupt.c

Let’s dive into a **comprehensive, step-by-step explanation** of the code. I’ll break it down into sections, explain each part in simple terms, and provide examples and diagrams where necessary. By the end, you’ll have a clear understanding of how the code works, even if you’re new to programming.

---

### **1. Header Files**
```c
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/gpio.h>     //GPIO
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/ktime.h>
```

#### **What It Does**
These lines include **header files** from the Linux kernel. Header files contain declarations of functions, macros, and data structures that the code needs to interact with the kernel and hardware.

#### **Breakdown**
- **`linux/kernel.h`**: Provides basic kernel functions and macros.
- **`linux/init.h`**: Defines macros for module initialization and cleanup.
- **`linux/module.h`**: Required for writing kernel modules.
- **`linux/kdev_t.h`**: Defines device number types.
- **`linux/fs.h`**: File system support, used for device file operations.
- **`linux/cdev.h`**: Character device support.
- **`linux/device.h`**: Device model support.
- **`linux/delay.h`**: Functions for delaying execution (e.g., `msleep()`).
- **`linux/uaccess.h`**: Functions for copying data between user and kernel space.
- **`linux/gpio.h`**: GPIO (General Purpose Input/Output) support.
- **`linux/interrupt.h`**: Interrupt handling support.
- **`linux/err.h`**: Error handling macros.
- **`linux/ktime.h`**: Functions for working with kernel time (e.g., `ktime_get_ns()`).

#### **Why These Are Used**
These headers are necessary because the code interacts with hardware (GPIO, UART) and the Linux kernel. Without them, the compiler wouldn’t know about the functions and data structures used in the code.

---

### **2. Constants and Global Variables**
```c
#define UART5_BASE 0xFE201A00 // 0xFE201A00 : Virtual Address
#define BLOCK_SIZE 4096
volatile unsigned int *uart5_addr;
volatile unsigned int *uart5_dr;
volatile unsigned int *uart5_fr;
volatile unsigned int *uart5_ibrd;
volatile unsigned int *uart5_fbrd;

char uart_buff[100];
```

#### **What It Does**
This section defines **constants** and **global variables** used throughout the code.

#### **Breakdown**
- **`UART5_BASE`**: The base address of UART5 in memory. This is where the UART hardware registers are located.
- **`BLOCK_SIZE`**: The size of the memory block to map (4096 bytes, which is one memory page).
- **`volatile unsigned int *uart5_addr`**: A pointer to the base address of UART5. The `volatile` keyword tells the compiler that this value can change at any time (e.g., by hardware).
- **`uart5_dr`, `uart5_fr`, `uart5_ibrd`, `uart5_fbrd`**: Pointers to specific UART registers:
  - **`uart5_dr`**: Data register (used to send/receive data).
  - **`uart5_fr`**: Flag register (used to check UART status).
  - **`uart5_ibrd`**: Integer baud rate divisor register.
  - **`uart5_fbrd`**: Fractional baud rate divisor register.
- **`char uart_buff[100]`**: A buffer to store data for UART communication.

#### **Why These Are Used**
- **`UART5_BASE`**: The UART hardware is accessed through memory-mapped I/O, so we need its base address.
- **`volatile`**: Ensures the compiler doesn’t optimize away reads/writes to hardware registers, which can change unexpectedly.
- **`uart_buff`**: Used to store strings or data before sending them over UART.

---

### **3. UART Initialization**
```c
void uart5_init(void)
{
    uart5_addr = ioremap(UART5_BASE, BLOCK_SIZE);
    uart5_dr = uart5_addr + 0x00;
    uart5_fr = uart5_addr + 0x18/4;
    uart5_ibrd = uart5_addr + 0x24/4;
    uart5_fbrd = uart5_addr + 0x28/4;    
    
    *uart5_ibrd = 26;
    *uart5_fbrd = 3;
}
```

#### **What It Does**
This function initializes UART5 by:
1. Mapping the UART hardware registers to kernel memory.
2. Setting the baud rate to 115,200 bps.

#### **Breakdown**
1. **`ioremap(UART5_BASE, BLOCK_SIZE)`**:
   - Maps the physical address `UART5_BASE` to a virtual address in kernel memory.
   - This allows the kernel to access the UART hardware registers.

2. **Pointer Arithmetic**:
   - `uart5_dr = uart5_addr + 0x00`: Points to the data register.
   - `uart5_fr = uart5_addr + 0x18/4`: Points to the flag register (offset 0x18, divided by 4 because pointers are 4 bytes).
   - Similar for `uart5_ibrd` and `uart5_fbrd`.

3. **Baud Rate Configuration**:
   - The baud rate is calculated as:
     ```
     BAUDDIV = (UART Clock) / (16 * Baud Rate)
     ```
     For 48 MHz clock and 115,200 bps:
     ```
     BAUDDIV = 48,000,000 / (16 * 115,200) = 26.041
     ```
   - **`*uart5_ibrd = 26`**: Sets the integer part of the divisor.
   - **`*uart5_fbrd = 3`**: Sets the fractional part (0.041 * 64 ≈ 3).

#### **Why This Is Used**
- **`ioremap()`**: Necessary to access hardware registers in kernel space.
- **Baud Rate Configuration**: Ensures the UART communicates at the correct speed.

---

### **4. UART Communication Functions**
```c
void uart_send_char(char data)
{
    while(*uart5_fr & (0x01 << 5));  // 5 : TXFE(if fifo enabled)
    *uart5_dr = data;    
}

void uart_send_str(char *str)
{
    int i;
    int str_len = strlen(str);
    for(i=0;i<str_len;i++)
    {
        uart_send_char(str[i]);
    }
}
```

#### **What It Does**
These functions send data over UART:
- **`uart_send_char()`**: Sends a single character.
- **`uart_send_str()`**: Sends a string by calling `uart_send_char()` for each character.

#### **Breakdown**
1. **`uart_send_char()`**:
   - **`while(*uart5_fr & (0x01 << 5))`**: Waits until the UART transmit FIFO is not full (bit 5 of the flag register).
   - **`*uart5_dr = data`**: Writes the character to the data register.

2. **`uart_send_str()`**:
   - **`strlen(str)`**: Gets the length of the string.
   - **`for(i=0;i<str_len;i++)`**: Loops through each character in the string.
   - **`uart_send_char(str[i])`**: Sends each character.

#### **Why This Is Used**
- **Polling**: The code waits for the UART to be ready before sending data, ensuring no data is lost.
- **Looping**: Sending a string character-by-character is simple and effective.

---

### **5. GPIO Interrupt Handling (Not Shown)**
The code snippet doesn’t include the GPIO interrupt handling logic, but it would typically involve:
1. Configuring a GPIO pin as an input.
2. Setting up an interrupt handler to respond to state changes.
3. Implementing debouncing to filter out false triggers.

---

### **Summary**
This code demonstrates how to:
1. Initialize UART hardware and configure its baud rate.
2. Send data over UART using polling.
3. Use memory-mapped I/O to interact with hardware registers.

Each part of the code is carefully designed to interact with hardware efficiently and reliably. By breaking it down step-by-step, we can see how the pieces fit together to achieve the overall functionality.