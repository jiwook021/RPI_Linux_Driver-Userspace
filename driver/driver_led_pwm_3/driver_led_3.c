// kernel header install
// sudo apt-get install raspberrypi-kernel-headers
//sudo mknod /dev/gpioled3 c 201 0
//=======================================
// helloLed.c
// hello + LED module
//=======================================
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/io.h> // ioremap(), iounmap()
#include <asm/uaccess.h> // copy_to_user(), copy_from_user()
#include <linux/ioport.h>
#include <linux/fs.h> // open(), close(), read(), write()
#include <linux/cdev.h>
// for BCM2711 GPIO Physical address : 0x7E200000
#define GPIO_BASE 0xFE200000 // 0xFE200000 : Virtual Address
#define BLOCK_SIZE 4096
#define GPIO_LED 18 // BCD_GPIO #18
#define MOD_NAME "gpioled3"
volatile unsigned int *gpio_addr;
volatile unsigned int *gpioset;
volatile unsigned int *gpioclr;

int led2_open(struct inode *minode, struct file *mfile) {
        printk("Kernel Module Open(): %s\n", MOD_NAME);
        return 0;
}

int led2_release(struct inode *minode, struct file *mfile) {
        printk("Kernel Module close(): %s\n", MOD_NAME);
        return 0;
}

ssize_t led2_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
        printk("Kernel Module write(): %s\n", MOD_NAME);
        unsigned char c;
        get_user(c, gdata); // if c==1 then ON, else OFF
        if(c==1)
        {
                unsigned int set_value = ioread32(gpioset);
                set_value |= 1 << (GPIO_LED);
                iowrite32(set_value, gpioset);
        }
        else if(c==0)
        {
                unsigned int clear_value = ioread32(gpioclr);
                clear_value |= 1 << (GPIO_LED);
                iowrite32(clear_value, gpioclr);
        }
        return 0;
}
static struct file_operations gpioled_fops = {
        .write = led2_write,
        .open = led2_open,
        .release = led2_release,
};

static int led_init(void) {
        printk("insmod : driver_led3 Module\n");
        int result;
        result = register_chrdev(201, MOD_NAME, &gpioled_fops);
        if(result < 0) {
                printk("Can't get any major\n");
                return result;
        }
        gpio_addr = ioremap(GPIO_BASE, BLOCK_SIZE);
        gpioset = ioremap(GPIO_BASE + 0x1C, 128);
        gpioclr = gpio_addr +0x28/4;
        *(gpio_addr+1) = 0x01000000;
        return 0;
}

static void led_exit(void) {
        iounmap(gpio_addr);
        unregister_chrdev(201, MOD_NAME);
        printk("rmmod : driver_led Module\n");
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Justin Kim jiwook021@gmail.com");
module_init(led_init);
module_exit(led_exit);
