// sudo mknod /dev/gpioled c 201 0

#include <asm/uaccess.h> // copy_to_user(), copy_from_user()
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h> // open(),close(), read(), write()
#include <linux/cdev.h>
#include <linux/io.h> // ioremap(), iounmap()
#include <linux/gpio.h> // *)!!!!

//#define MOD_MAJOR 0 // automatic allocation
#define MOD_MAJOR 201
#define MOD_NAME "gpioled"
#define GPIO_LED 18 // BCM_GPIO #18

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
    
    get_user(c, gdata);
    gpio_set_value(GPIO_LED, ((c == 0) ? 0 : 1));
    
    return length;
}

static struct file_operations gpioled_fops = {
    .write = gpioled_write,
    .open = gpioled_open,
    .release = gpioled_release,
};

int gpioled_init(void) {
    int result;
    
    result=register_chrdev(MOD_MAJOR,MOD_NAME,&gpioled_fops);
    
    if(result < 0) {
        printk("Can't get any major\n");
        return result;
    }
    
    printk("Init Module: Major number %d\n", MOD_MAJOR);
    
    gpio_request(GPIO_LED, "LED");
    gpio_direction_output(GPIO_LED, 0);
    
    return 0;
}

void gpioled_exit(void) {
    unregister_chrdev(MOD_MAJOR, MOD_NAME);
    printk("%s DRIVER CLEANUP\n", MOD_NAME);
    gpio_free(GPIO_LED);
}

module_init(gpioled_init);
module_exit(gpioled_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("melee");
MODULE_DESCRIPTION("Raspberry Pi 3 GPIO LED Device Driver Module");
