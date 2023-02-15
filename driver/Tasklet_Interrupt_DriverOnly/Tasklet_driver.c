
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/interrupt.h>
//#include <asm/io.h>
#include <linux/err.h>
#include <linux/gpio.h>//GPIO
#include <linux/delay.h>//sleep

//#define IRQ_NO 11

#define MOD_MAJOR 204
#define MOD_NAME "Tasklet"

static int btn_irq;

#define GPIO_BTN 17 // BCM_GPIO #17
#define GPIO_LED 18 // BCM_GPIO #18


void tasklet_fn(unsigned long);
/* Tasklet by Dynamic Method */
struct tasklet_struct* tasklet = NULL;


/*Tasklet Function*/
void tasklet_fn(unsigned long arg)
{
    int i;
    for (i = 0; i < 10; i++)
    {
        gpio_set_value(GPIO_LED, 1);
        msleep(500);
        gpio_set_value(GPIO_LED, 0);
        msleep(500);
    }
    printk(KERN_INFO "Executing Tasklet Function : arg = %ld\n", arg);
}


//Interrupt handler for IRQ 11. 
static irqreturn_t irq_handler(int irq, void* dev_id) {
    printk(KERN_INFO "Shared IRQ: Interrupt Occurred");
    /*Scheduling Task to Tasklet*/
    tasklet_schedule(tasklet);

    return IRQ_HANDLED;
}


volatile int Tasklet_value = 0;


dev_t dev = 0;
static struct class* dev_class;
static struct cdev Tasklet_cdev;
struct kobject* kobj_ref;

static int __init Tasklet_driver_init(void);
static void __exit Tasklet_driver_exit(void);

/*************** Driver Functions **********************/
static int Tasklet_open(struct inode* inode, struct file* file);
static int Tasklet_release(struct inode* inode, struct file* file);
static ssize_t Tasklet_read(struct file* filp,
    char __user* buf, size_t len, loff_t* off);
static ssize_t Tasklet_write(struct file* filp,
    const char* buf, size_t len, loff_t* off);

/*************** Sysfs Functions **********************/
static ssize_t sysfs_show(struct kobject* kobj,
    struct kobj_attribute* attr, char* buf);
static ssize_t sysfs_store(struct kobject* kobj,
    struct kobj_attribute* attr, const char* buf, size_t count);

struct kobj_attribute Tasklet_attr = __ATTR(Tasklet_value, 0660, sysfs_show, sysfs_store);
/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner = THIS_MODULE,
        .read = Tasklet_read,
        .write = Tasklet_write,
        .open = Tasklet_open,
        .release = Tasklet_release,
};
/*
** This function will be called when we read the sysfs file
*/
static ssize_t sysfs_show(struct kobject* kobj,
    struct kobj_attribute* attr, char* buf)
{
    printk(KERN_INFO "Sysfs - Read!!!\n");
    return sprintf(buf, "%d", Tasklet_value);
}
/*
** This function will be called when we write the sysfsfs file
*/
static ssize_t sysfs_store(struct kobject* kobj,
    struct kobj_attribute* attr, const char* buf, size_t count)
{
    printk(KERN_INFO "Sysfs - Write!!!\n");
    sscanf(buf, "%d", &Tasklet_value);
    return count;
}
/*
** This function will be called when we open the Device file
*/
static int Tasklet_open(struct inode* inode, struct file* file)
{
    printk(KERN_INFO "Device File Opened...!!!\n");
    return 0;
}

/*
** This function will be called when we close the Device file
*/
static int Tasklet_release(struct inode* inode, struct file* file)
{
    printk(KERN_INFO "Device File Closed...!!!\n");
    return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t Tasklet_read(struct file* filp,
    char __user* buf, size_t len, loff_t* off)
{
    printk(KERN_INFO "Read function\n");
    //asm("int $0x3B");  // Corresponding to irq 11
    return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t Tasklet_write(struct file* filp,
    const char __user* buf, size_t len, loff_t* off)
{
    printk(KERN_INFO "Write Function\n");
    return len;
}

/*
** Module Init function
*/
static int __init Tasklet_driver_init(void)
{
    /*Allocating Major number*/
    if ((alloc_chrdev_region(&dev, 3, 1, "Tasklet_Dev")) < 0) {
        printk(KERN_INFO "Cannot allocate major number\n");
        return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&Tasklet_cdev, &fops);

    /*Adding character device to the system*/
    if ((cdev_add(&Tasklet_cdev, dev, 1)) < 0) {
        printk(KERN_INFO "Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creating struct class*/
    if (IS_ERR(dev_class = class_create(THIS_MODULE, "Tasklet_class"))) {
        printk(KERN_INFO "Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "Tasklet_device"))) {
        printk(KERN_INFO "Cannot create the Device 1\n");
        goto r_device;
    }

    /*Creating a directory in /sys/kernel/ */
    kobj_ref = kobject_create_and_add("Tasklet_sysfs", kernel_kobj);

    /*Creating sysfs file for Tasklet_value*/
    if (sysfs_create_file(kobj_ref, &Tasklet_attr.attr)) {
        printk(KERN_INFO"Cannot create sysfs file......\n");
        goto r_sysfs;
    }
    gpio_request(GPIO_BTN, "SWITCH_WORKQUEUE");
    gpio_direction_input(GPIO_BTN);

    gpio_request(GPIO_LED, "LED_WORKQUEUE");
    gpio_direction_output(GPIO_LED, 0);

    btn_irq = gpio_to_irq(GPIO_BTN);

    if (request_irq(btn_irq, irq_handler, IRQF_TRIGGER_FALLING, "SWITCH_WORKQUEUE", NULL)) {
        printk(KERN_INFO "Tasklet_device: cannot register IRQ ");
        goto irq;
    }
    /* Init the tasklet bt Dynamic Method */
    tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
    if (tasklet == NULL) {
        printk(KERN_INFO "Tasklet_device: cannot allocate Memory");
        goto irq;
    }
    tasklet_init(tasklet, tasklet_fn, 0);

 

    
    printk(KERN_INFO "Device Driver Insert...Done!!!\n");

    
    return 0;

irq:
    free_irq(btn_irq, (void*)(irq_handler));

r_sysfs:
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &Tasklet_attr.attr);

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    cdev_del(&Tasklet_cdev);
    return -1;
}
/*
** Module exit function
*/
static void __exit Tasklet_driver_exit(void)
{
    /* Kill the Tasklet */
    tasklet_kill(tasklet);
    if (tasklet != NULL)
    {
        kfree(tasklet);
    }
    free_irq(btn_irq, (void*)(irq_handler));
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &Tasklet_attr.attr);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&Tasklet_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}

module_init(Tasklet_driver_init);
module_exit(Tasklet_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Justin Kim jiwook021@gmail.com");
MODULE_DESCRIPTION("A simple device driver - Tasklet Dynamic");