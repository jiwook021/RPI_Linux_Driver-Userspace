


#include <asm/uaccess.h> // copy_to_user(), copy_from_user()
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h> // open(), close(), read(), write()
#include <linux/cdev.h>
#include <linux/io.h> // ioremap(), iounmap()


MODULE_LICENSE("GPL");
MODULE_AUTHOR("HA");


// for BCM2711 GPIO Physical address : 0x7E200000
#define GPIO_BASE 0xFE200000 // 0xFE200000 : Virtual Address

#define GPIO_RANGE 1024
// address offset of registers for BCM_GPIO #18
#define GPFSEL0 (0x00/4) // int *
#define GPSET0 (0x1C/4)
#define GPCLR0 (0x28/4)
//#define MOD_MAJOR 0 // automatic allocation
#define MOD_MAJOR 201
#define MOD_NAME "gpioled"

#define GPIO_LED 18 // BCM_GPIO #18
//#define GPIO_LED 17 // BCM_GPIO #17

volatile unsigned int * gpio_addr;

int gpioled_open(struct inode *minode, struct file *mfile) {
        printk("Kernel Module Open(): %s\n", MOD_NAME);
        return 0;
}

int gpioled_release(struct inode *minode, struct file *mfile) {
        printk("Kernel Module close(): %s\n", MOD_NAME);
        return 0;
}

ssize_t gpioled_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
        unsigned char c;
        get_user(c, gdata); // if c==1 then ON, else OFF
        if (c == 1)
        *(gpio_addr+GPSET0) |= 1 << (GPIO_LED); // ON
        else
        *(gpio_addr+GPCLR0) |= 1 << (GPIO_LED); // OFF
        return length;
}

static struct file_operations gpioled_fops = {
        .write = gpioled_write,
        .open = gpioled_open,
        .release = gpioled_release,
};


int gpioled_init(void) {
        int result;
        result = register_chrdev(MOD_MAJOR, MOD_NAME, &gpioled_fops);
        if(result < 0) {
                printk("Can't get any major\n");
                return result;
        }

        printk("Init Module: Major number %d\n", MOD_MAJOR);
        printk("Init Module: Major number %d\n", result);

        gpio_addr = ioremap(GPIO_BASE, GPIO_RANGE);
        // GPFSELn, #18, out
        *(gpio_addr+(GPIO_LED/10)) |= (7 << (((GPIO_LED)%10)*3));
        *(gpio_addr+(GPIO_LED/10)) &= ~(6 << (((GPIO_LED)%10)*3));

        return 0;
}

void gpioled_exit(void) {
        unregister_chrdev(MOD_MAJOR, MOD_NAME);
        printk("%s DRIVER CLEANUP\n", MOD_NAME);
}

module_init(gpioled_init);
module_exit(gpioled_exit);
