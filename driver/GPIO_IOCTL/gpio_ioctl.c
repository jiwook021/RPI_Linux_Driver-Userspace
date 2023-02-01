#include <asm/uaccess.h> // copy_to_user(), copy_from_user()
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h> // open(),close(), read(), write()
#include <linux/cdev.h>
#include <linux/io.h> // ioremap(), iounmap()
#include <linux/gpio.h> // *)!!!!
#include <linux/ioctl.h>

//#define MOD_MAJOR 0 // automatic allocation
#define MOD_MAJOR 201
#define MOD_NAME "gpioled"
#define GPIO_LED 18 // BCM_GPIO #18


//sudo mknod /dev/gpioled c 201 0
//sudo chmod 777 /dev/gpioled




#define  LED_DEV_MAGIC   'Y'
 
#define  LED_INIT    _IO(LED_DEV_MAGIC,   0)
#define  LED_ON      _IOW(LED_DEV_MAGIC, 1, unsigned char)
#define  LED_OFF     _IOW(LED_DEV_MAGIC, 2, unsigned char) 


int gpioled_open(struct inode *minode, struct file *mfile) {
    printk("Kernel Module Open(): %s\n", MOD_NAME);
    return 0;
}

int gpioled_release(struct inode *minode, struct file *mfile) {
    printk("Kernel Module close(): %s\n", MOD_NAME);
    return 0;
}

ssize_t gpioled_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
    unsigned char c;
    
    get_user(c, gdata);
    //put_user 

    //copy from user, copy to from. 
    gpio_set_value(GPIO_LED, ((c == 0) ? 0 : 1));
    
    return length;
}


long int lcd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
 
    switch(cmd) 
    {
    case LED_INIT :
        printk("LED INIT..\n");
        break;
    case LED_ON :
        printk("LED ON = %02x..\n",(unsigned char )arg);
        gpio_set_value(GPIO_LED, (1));
        break;
    case LED_OFF :
        printk("LED OFF = %02x..\n",(unsigned char )arg);
        gpio_set_value(GPIO_LED, (0));
        break;
    default :
        printk("ioctl command mismatch ERROR! \n");
    }
    return 0;
}

static struct file_operations gpioled_fops = {
    .write = gpioled_write,
    .open = gpioled_open,
    .release = gpioled_release,
    .unlocked_ioctl = lcd_ioctl,
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