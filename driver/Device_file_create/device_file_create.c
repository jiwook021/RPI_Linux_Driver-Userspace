
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/gpio.h> 
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev haha_cdev;
char *msg="haha..";

#define  GPIO_LED 18
#define  LED_DEV_MAGIC   'Y'
 
#define  LED_INIT    _IO(LED_DEV_MAGIC,   0)
#define  LED_ON      _IOW(LED_DEV_MAGIC, 1, unsigned char)
#define  LED_OFF     _IOW(LED_DEV_MAGIC, 2, unsigned char) 


int gpioled_open(struct inode *minode, struct file *mfile) 
{
    printk("Kernel Module Open(): %s\n");
    return 0;
}

int gpioled_release(struct inode *minode, struct file *mfile) 
{
    printk("Kernel Module close(): %s\n");
    return 0;
}

ssize_t gpioled_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
    unsigned char c;
    
    get_user(c, gdata);
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
        gpio_set_value(GPIO_LED, 1);
        break;
    case LED_OFF :
        printk("LED OFF = %02x..\n",(unsigned char )arg);
        gpio_set_value(GPIO_LED, 0);
        break;
    default :
        printk("ioctl command mismatch ERROR! \n");
    }
    return 0;
}

static struct file_operations haha_fops = {
    .write = gpioled_write,
    .open = gpioled_open,
    .release = gpioled_release,
    .unlocked_ioctl = lcd_ioctl,
};

static int __init haha_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 2, 2, "dev_file_create")) <0)
        {
                pr_err("Cannot allocate major number for device\n");
                return -1;
        }
        pr_info("device_file_create : Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

        /*Creating cdev structure*/
        cdev_init(&haha_cdev,&haha_fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&haha_cdev,dev,1)) < 0){
            pr_info("Cannot add the device to the system\n");
        }

        /*Creating struct class*/
        dev_class = class_create(THIS_MODULE,"haha_class");
        if(IS_ERR(dev_class)){
            pr_err("Cannot create the struct class for device\n");
            goto r_class;
        }
 
        /*Creating device*/
        if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"device_file_create"))){
            pr_err("Cannot create the Device\n");
            goto r_device;
        }
        pr_info("Kernel Module Inserted Successfully...\n");
        return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}
 
/*
** Module exit function
*/
static void __exit haha_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&haha_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Kernel Module Removed Successfully...\n");
}
 
module_init(haha_init);
module_exit(haha_exit);
 
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple linux driver (Automatically Creating a Device file)");
MODULE_VERSION("1.2");

// static struct cdev haha_cdev;

// ==================================

//         /*Creating cdev structure*/
//         cdev_init(&haha_cdev,&fops);
 
//         /*Adding character device to the system*/
//         if((cdev_add(&haha_cdev,dev,1)) < 0){
//             pr_info("Cannot add the device to the system\n");
//         }

// ==================================

// cdev_del(&haha_cdev);