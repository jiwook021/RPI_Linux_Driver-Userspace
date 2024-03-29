// sysfs_driver.c



// sudo insmod sysfs_driver.ko
// root execute ( sudo su )
// sudo echo 123 > /sys/kernel/haha_sysfs/haha_value
// sudo echo 1 > /sys/kernel/haha_sysfs/haha_value
// sudo cat /sys/kernel/haha_sysfs/haha_value


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
#include <linux/err.h>
#include <linux/gpio.h> // *)!!!!
 
volatile int haha_value = 0;
 
#define GPIO_LED 18
#define GPIO_BTN 17 // BCM_GPIO #17

dev_t dev = 0;
static struct class *dev_class;
static struct cdev haha_cdev;
struct kobject *kobj_ref;
/*
** Function Prototypes
*/
static int      __init haha_driver_init(void);
static void     __exit haha_driver_exit(void);
 
/*************** Driver functions **********************/
static int      haha_open(struct inode *inode, struct file *file);
static int      haha_release(struct inode *inode, struct file *file);
static ssize_t  haha_read(struct file *filp, 
                        char __user *buf, size_t len,loff_t * off);
static ssize_t  haha_write(struct file *filp, 
                        const char *buf, size_t len, loff_t * off);
 
/*************** Sysfs functions **********************/
static ssize_t  sysfs_show(struct kobject *kobj, 
                        struct kobj_attribute *attr, char *buf);
static ssize_t  sysfs_store(struct kobject *kobj, 
                        struct kobj_attribute *attr,const char *buf, size_t count);
struct kobj_attribute haha_attr = __ATTR(haha_value, 0660, sysfs_show, sysfs_store);
/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = haha_read,
        .write          = haha_write,
        .open           = haha_open,
        .release        = haha_release,
};
/*
** This function will be called when we read the sysfs file
*/
static ssize_t sysfs_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
        pr_info("Sysfs - Read!!!%d \n", haha_value);
        

        haha_value = gpio_get_value(GPIO_BTN);

        
        return sprintf(buf, "%d", haha_value); 
        
}
/*
** This function will be called when we write the sysfsfs file
*/
static ssize_t sysfs_store(struct kobject *kobj, 
                struct kobj_attribute *attr,const char *buf, size_t count)
{
        
        sscanf(buf,"%d",&haha_value);
        //input 1 on 0 off
        //
        gpio_set_value(GPIO_LED, ((haha_value == 0) ? 0 : 1));
        pr_info("Sysfs - Write!!! = %d \n", haha_value);
        

        return count;
}
/*
** This function will be called when we open the Device file
*/ 
static int haha_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
/*
** This function will be called when we close the Device file
*/ 
static int haha_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
 
/*
** This function will be called when we read the Device file
*/
static ssize_t haha_read(struct file *filp, 
                char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read function\n");
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t haha_write(struct file *filp, 
                const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write Function\n");
        return len;
}
 
/*
** Module Init function
*/
static int __init haha_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "haha_Dev")) <0){
                pr_info("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&haha_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&haha_cdev,dev,1)) < 0){
            pr_info("Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if(IS_ERR(dev_class = class_create(THIS_MODULE,"haha_class"))){
            pr_info("Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"haha_device"))){
            pr_info("Cannot create the Device 1\n");
            goto r_device;
        }
 
        /*Creating a directory in /sys/kernel/ */
        kobj_ref = kobject_create_and_add("haha_sysfs",kernel_kobj);
 
        /*Creating sysfs file for haha_value*/
        if(sysfs_create_file(kobj_ref,&haha_attr.attr)){
                pr_err("Cannot create sysfs file......\n");
                goto r_sysfs;
    }
        pr_info("Device Driver Insert...Done!!!\n");

        gpio_request(GPIO_LED, "LED");
        gpio_direction_output(GPIO_LED, 0);

        gpio_request(GPIO_BTN, "SWITCH");
        gpio_direction_input(GPIO_BTN);

        return 0;
 
r_sysfs:
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &haha_attr.attr);
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        cdev_del(&haha_cdev);
        return -1;
}
/*
** Module exit function
*/
static void __exit haha_driver_exit(void)
{
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &haha_attr.attr);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&haha_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(haha_driver_init);
module_exit(haha_driver_exit);
 
MODULE_AUTHOR("Justin Kim jiwook021@gmail.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Linux device driver (sysfs)");
MODULE_VERSION("1");


