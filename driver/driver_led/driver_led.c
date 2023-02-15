// kernel header install
// sudo apt-get install raspberrypi-kernel-headers


//=======================================
// helloLed.c
// hello + LED module
//=======================================
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/io.h> // ioremap(), iounmap()



// for BCM2711 GPIO Physical address : 0x7E200000
#define GPIO_BASE 0xFE200000 // 0xFE200000 : Virtual Address

#define BLOCK_SIZE 4096

#define GPIO_LED 18 // BCD_GPIO #18
//#define GPIO_LED 17 // BCD_GPIO #17
#define GPFSEL1 (0x04/4)
#define GPSET0 (0x1C/4)
#define GPCLR0 (0x28/4)

volatile unsigned int * gpio_addr;

// LED blinking
static void blink(void) {
        int i;
        for(i=0; i<5 ; i++){
                printk("haha\n");
                *(gpio_addr+GPSET0) |= 1 << (GPIO_LED);
                mdelay(1000);
                *(gpio_addr+GPCLR0) |= 1 << (GPIO_LED);
                mdelay(1000);
        }
}

static void blink1(void) {
        int i;
        for(i=0; i<10 ; i++){
                unsigned int set_value = ioread32(gpio_addr+GPSET0);
                set_value |= 1 << (GPIO_LED);
                iowrite32(set_value, gpio_addr+GPSET0);
                mdelay(500);
            
                unsigned int clear_value = ioread32(gpio_addr+GPCLR0);
                clear_value |= 1 << (GPIO_LED);
                iowrite32(clear_value, gpio_addr+GPCLR0);
                mdelay(500);
        }
}


static int led_init(void) {

        printk("insmod : driver_led Module\n");
        gpio_addr = ioremap(GPIO_BASE, BLOCK_SIZE);

        *(gpio_addr+GPFSEL1) &= ~(0x07 << 24);
        *(gpio_addr+GPFSEL1) |= (0x01 << 24);

        //blink();
        blink1();

        return 0;
}

static void led_exit(void) {
        iounmap(gpio_addr);
        printk("rmmod : driver_led Module\n");
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Justin Kim jiwook021@gmail.com");
module_init(led_init);
module_exit(led_exit);
