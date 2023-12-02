/***************************************************************************//**
*  \file       driver.c
*
*  \details    Simple Linux device driver (Signals)
*
*  \author     EmbeTronicX
*
* *******************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/err.h>

#define SIGETX 44 
#define REG_CURRENT_TASK _IOW('a','a',int32_t*)
#define IRQ_NO 11
 
/* Signaling to Application */
static struct task_struct *task = NULL;
static int signum = 0;
 
int32_t value = 0;
dev_t dev = 0;
static struct class *dev_class;
static struct cdev sample_cdev;
 
static int __init sample_driver_init(void);
static void __exit sample_driver_exit(void);
static int sample_open(struct inode *inode, struct file *file);
static int sample_release(struct inode *inode, struct file *file);
static ssize_t sample_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t sample_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long sample_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

struct timer_list timer;

void kernel_timer_func(struct timer_list *);
void kernel_timer_func(struct timer_list *timer)
{
    struct kernel_siginfo info;
    printk(KERN_INFO "Shared IRQ: Interrupt Occurred");
    //Sending signal to app
    memset(&info, 0, sizeof(struct kernel_siginfo));//kernel_siginfo
    info.si_signo = SIGETX;
    info.si_code = SI_QUEUE;
    info.si_int = 10;
    if (task != NULL) {
        printk(KERN_INFO "Sending signal to app\n");
        if(send_sig_info(SIGETX, &info, task) < 0) {
            printk(KERN_INFO "Unable to send signal\n");
        }
    }
    mod_timer(timer, get_jiffies_64() + 100);
}

static void kernel_timer_register(void)
{
    timer_setup(&timer, kernel_timer_func, 0);
    timer.expires = get_jiffies_64() + 100;
    add_timer(&timer);
}
 
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = sample_read,
        .write          = sample_write,
        .open           = sample_open,
        .unlocked_ioctl = sample_ioctl,
        .release        = sample_release,
};
 
//Interrupt handler for IRQ 11. change to kernel timer
// static irqreturn_t irq_handler(int irq,void *dev_id) {
//     struct siginfo info;
//     printk(KERN_INFO "Shared IRQ: Interrupt Occurred");
    
//     //Sending signal to app
//     memset(&info, 0, sizeof(struct siginfo));//kernel_siginfo
//     info.si_signo = SIGETX;
//     info.si_code = SI_QUEUE;
//     info.si_int = 1;
 
//     if (task != NULL) {
//         printk(KERN_INFO "Sending signal to app\n");
//         if(send_sig_info(SIGETX, &info, task) < 0) {
//             printk(KERN_INFO "Unable to send signal\n");
//         }
//     }
 
//     return IRQ_HANDLED;
// }
 
static int sample_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device File Opened...!!!\n");
    return 0;
}
 
static int sample_release(struct inode *inode, struct file *file)
{
    struct task_struct *ref_task = get_current();
    printk(KERN_INFO "Device File Closed...!!!\n");
    //delete the task
    if(ref_task == task) {
        task = NULL;
    }
    return 0;
}
 
static ssize_t sample_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Read Function\n");
    //asm("int $0x3B");  //Triggering Interrupt. Corresponding to irq 11 x86 
    //kernel timer sends signal in every one second.  
    return 0;
}

static ssize_t sample_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Write function\n");
    return 0;
}
 
static long sample_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if (cmd == REG_CURRENT_TASK) {
        printk(KERN_INFO "REG_CURRENT_TASK\n");
        task = get_current();
        signum = SIGETX;
    }
    return 0;
}
 
 
static int __init sample_driver_init(void)
{
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "sample_Dev")) <0){
            printk(KERN_INFO "Cannot allocate major number\n");
            return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
    /*Creating cdev structure*/
    cdev_init(&sample_cdev,&fops);
    /*Adding character device to the system*/
    if((cdev_add(&sample_cdev,dev,1)) < 0){
        printk(KERN_INFO "Cannot add the device to the system\n");
        goto r_class;
    }
    /*Creating struct class*/
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"sample_class"))){
        printk(KERN_INFO "Cannot create the struct class\n");
        goto r_class;
    }
    /*Creating device*/
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"sample_device"))){
        printk(KERN_INFO "Cannot create the Device 1\n");
        goto r_device;
    }
    // if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "sample_device", (void *)(irq_handler))) {
    //     printk(KERN_INFO "my_device: cannot register IRQ ");
    //     goto irq;
    // }
    kernel_timer_register();
    printk(KERN_INFO "Device Driver Insert...Done!!!\n");
    return 0;
irq:
    //free_irq(IRQ_NO,(void *)(irq_handler));
    del_timer(&timer);
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}
 
static void __exit sample_driver_exit(void)
{
    // free_irq(IRQ_NO,(void *)(irq_handler));
    del_timer(&timer);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&sample_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}
 
module_init(sample_driver_init);
module_exit(sample_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - Signals");
MODULE_VERSION("1.20");