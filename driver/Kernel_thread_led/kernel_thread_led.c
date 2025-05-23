#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/kthread.h>             //kernel threads
#include <linux/sched.h>               //task_struct 
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>//GPIO

#define GPIO_LED 18 // BCM_GPIO #18

dev_t dev = 0;
static struct class* dev_class;
static struct cdev led_thread_cdev;

static int __init led_thread_driver_init(void);
static void __exit led_thread_driver_exit(void);

static struct task_struct* led_thread_thread;

/*
** Function Prototypes
*/
/*************** Driver functions **********************/
static int led_thread_open(struct inode* inode, struct file* file);
static int led_thread_release(struct inode* inode, struct file* file);
static ssize_t led_thread_read(struct file* filp,
    char __user* buf, size_t len, loff_t* off);
static ssize_t led_thread_write(struct file* filp,
    const char* buf, size_t len, loff_t* off);
/******************************************************/

int thread_function(void* pv);
/*
** Thread
*/
int thread_function(void* pv)
{
    int i = 0;
    while (!kthread_should_stop()) {
       
            gpio_set_value(GPIO_LED, 1);
            msleep(500);
            gpio_set_value(GPIO_LED, 0);
            msleep(500);
        

    }
    return 0;
}
/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner = THIS_MODULE,
        .read = led_thread_read,
        .write = led_thread_write,
        .open = led_thread_open,
        .release = led_thread_release,
};
/*
** This function will be called when we open the Device file
*/
static int led_thread_open(struct inode* inode, struct file* file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}
/*
** This function will be called when we close the Device file
*/
static int led_thread_release(struct inode* inode, struct file* file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t led_thread_read(struct file* filp,
    char __user* buf, size_t len, loff_t* off)
{
    pr_info("Read function\n");

    return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t led_thread_write(struct file* filp,
    const char __user* buf, size_t len, loff_t* off)
{
    pr_info("Write Function\n");
    return len;
}
/*
** Module Init function
*/
static int __init led_thread_driver_init(void)
{
    /*Allocating Major number*/
    if ((alloc_chrdev_region(&dev, 4, 1, "led_thread_Dev")) < 0) {
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&led_thread_cdev, &fops);

    /*Adding character device to the system*/
    if ((cdev_add(&led_thread_cdev, dev, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }


    gpio_request(GPIO_LED, "LED_THREAD");
    gpio_direction_output(GPIO_LED, 0);

    /*Creating struct class*/
    if (IS_ERR(dev_class = class_create(THIS_MODULE, "led_thread_class"))) {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "led_thread_device"))) {
        pr_err("Cannot create the Device \n");
        goto r_device;
    }

    led_thread_thread = kthread_create(thread_function, NULL, "led_thread Thread");
    if (led_thread_thread) {
        wake_up_process(led_thread_thread);
    }
    else {
        pr_err("Cannot create kthread\n");
        goto r_device;
    }

#if 0
    /* You can use this method also to create and run the thread */
    led_thread_thread = kthread_run(thread_function, NULL, "led_thread Thread");
    if (led_thread_thread) {
        pr_info("Kthread Created Successfully...\n");
    }
    else {
        pr_err("Cannot create kthread\n");
        goto r_device;
    }
#endif
    pr_info("Device Driver Insert...Done!!!\n");
    return 0;


r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    cdev_del(&led_thread_cdev);
    return -1;
}
/*
** Module exit function
*/
static void __exit led_thread_driver_exit(void)
{
    kthread_stop(led_thread_thread);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&led_thread_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!\n");
}

module_init(led_thread_driver_init);
module_exit(led_thread_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Justin Kim jiwook021@gmail.com");
MODULE_DESCRIPTION("A simple device driver - Kernel Thread");
MODULE_VERSION("1");