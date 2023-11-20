#define UART5_BASE 0xFE201A00 // 0xFE201A00 : Virtual Address
#define BLOCK_SIZE 4096
volatile unsigned int *uart5_addr;
volatile unsigned int *uart5_dr;
volatile unsigned int *uart5_fr;
volatile unsigned int *uart5_ibrd;
volatile unsigned int *uart5_fbrd;

char uart_buff[100];

void uart5_init(void)
{
    uart5_addr = ioremap(UART5_BASE, BLOCK_SIZE);
    uart5_dr = uart5_addr + 0x00;
    uart5_fr = uart5_addr + 0x18/4;
    uart5_ibrd = uart5_addr + 0x24/4;
    uart5_fbrd = uart5_addr + 0x28/4;    
    
    //------------------------------------
    // FBRD is a 6 bit number (0-63) to represent the fractional divisor.
    //-----------------
    // The uart clock is 48MHz, and we are going to use a fixed, 115,200 baud rate. So...
    // BAUDDIV = 48,000,000 / (16 * 115,200) = 26.041
    // IBRD = floor(26.041) =  26
    // FBRD = 0.041* 64   = 3(2.624)
    //-----------------
    /* default uart clock : 48 MHz, Baudrate : 115200 */
    *uart5_ibrd = 26;
    *uart5_fbrd = 3;
    //------------------------------------
}

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
//=================================================================================