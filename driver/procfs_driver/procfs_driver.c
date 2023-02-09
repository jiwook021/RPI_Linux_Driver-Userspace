// procfs_driver.c


// cd /proc/haha
// cat haha_proc
// sudo echo haha > haha_proc
// cat haha_proc
// /proc/haha/haha_proc

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include<linux/proc_fs.h>
#include <linux/err.h>
#include <linux/gpio.h>
/*
** I am using the kernel 5.10.27-v7l. So I have set this as 510.
** If you are using the kernel 3.10, then set this as 310,
** and for kernel 5.1, set this as 501. Because the API proc_create()
** changed in kernel above v5.5.
**
*/
#define LINUX_KERNEL_VERSION  510

#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

int32_t value = 0;
char haha_array[20] = "try_proc_array\n";
static int len = 1;


dev_t dev = 0;
static struct class* dev_class;
static struct cdev haha_cdev;
static struct proc_dir_entry* parent;
/*
** Function Prototypes
*/
static int      __init haha_driver_init(void);
static void     __exit haha_driver_exit(void);
/*************** Driver Functions **********************/
static int      haha_open(struct inode* inode, struct file* file);
static int      haha_release(struct inode* inode, struct file* file);
static ssize_t  haha_read(struct file* filp, char __user* buf, size_t len, loff_t* off);
static ssize_t  haha_write(struct file* filp, const char* buf, size_t len, loff_t* off);
static long     haha_ioctl(struct file* file, unsigned int cmd, unsigned long arg);

/***************** Procfs Functions *******************/
static int      open_proc(struct inode* inode, struct file* file);
static int      release_proc(struct inode* inode, struct file* file);
static ssize_t  read_proc(struct file* filp, char __user* buffer, size_t length, loff_t* offset);
static ssize_t  write_proc(struct file* filp, const char* buff, size_t len, loff_t* off);


#define GPIO_LED 18

/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner = THIS_MODULE,
        .read = haha_read,
        .write = haha_write,
        .open = haha_open,
        .unlocked_ioctl = haha_ioctl,
        .release = haha_release,
};
#if ( LINUX_KERNEL_VERSION > 505 )
/*
** procfs operation sturcture
*/
static struct proc_ops proc_fops = {
        .proc_open = open_proc,
        .proc_read = read_proc,
        .proc_write = write_proc,
        .proc_release = release_proc
};
#else //LINUX_KERNEL_VERSION > 505
/*
** procfs operation sturcture
*/
static struct file_operations proc_fops = {
        .open = open_proc,
        .read = read_proc,
        .write = write_proc,
        .release = release_proc
};
#endif //LINUX_KERNEL_VERSION > 505
/*
** This function will be called when we open the procfs file
*/
static int open_proc(struct inode* inode, struct file* file)
{
    pr_info("proc file opend.....\t");
    return 0;
}
/*
** This function will be called when we close the procfs file
*/
static int release_proc(struct inode* inode, struct file* file)
{
    pr_info("proc file released.....\n");
    return 0;
}
/*
** This function will be called when we read the procfs file
*/
static ssize_t read_proc(struct file* filp, char __user* buffer, size_t length, loff_t* offset)
{
    pr_info("proc file read.....\n");
    if (len)
    {
        len = 0;
    }
    else
    {
        len = 1;
        return 0;
    }

    if (copy_to_user(buffer, haha_array, 20))
    {
        pr_err("Data Send : Err!\n");
    }

    return length;;
}
/*
** This function will be called when we write the procfs file
*/
static ssize_t write_proc(struct file* filp, const char* buff, size_t len, loff_t* off)
{
    pr_info("proc file wrote.....\n");

    //1 on 0 off every 1 sec. 
    unsigned char c;

    get_user(c, buff);

    if(c=='0')
    {
        gpio_set_value(GPIO_LED, 0);
    }
    else
    {
        gpio_set_value(GPIO_LED, 1);
    }
    //gpio_set_value(GPIO_LED, ((c == 0) ? 0 : 1));



    if (copy_from_user(haha_array, buff, len))
    {
        pr_err("Data Write : Err!\n");
    }

    return len;
}
/*
** This function will be called when we open the Device file
*/
static int haha_open(struct inode* inode, struct file* file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}
/*
** This function will be called when we close the Device file
*/
static int haha_release(struct inode* inode, struct file* file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t haha_read(struct file* filp, char __user* buf, size_t len, loff_t* off)
{
    pr_info("Read function\n");
    return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t haha_write(struct file* filp, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("Write Function\n");
    return len;
}
/*
** This function will be called when we write IOCTL on the Device file
*/
static long haha_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case WR_VALUE:
        if (copy_from_user(&value, (int32_t*)arg, sizeof(value)))
        {
            pr_err("Data Write : Err!\n");
        }
        pr_info("Value = %d\n", value);
        break;
    case RD_VALUE:
        if (copy_to_user((int32_t*)arg, &value, sizeof(value)))
        {
            pr_err("Data Read : Err!\n");
        }
        break;
    default:
        pr_info("Default\n");
        break;
    }
    return 0;
}

/*
** Module Init function
*/
static int __init haha_driver_init(void)
{
    /*Allocating Major number*/
    if ((alloc_chrdev_region(&dev, 0, 1, "procfs_Dev")) < 0) {
        pr_info("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&haha_cdev, &fops);

    /*Adding character device to the system*/
    if ((cdev_add(&haha_cdev, dev, 1)) < 0) {
        pr_info("Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creating struct class*/
    if (IS_ERR(dev_class = class_create(THIS_MODULE, "procfs_class"))) {
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "procfs_device"))) {
        pr_info("Cannot create the Device 1\n");
        goto r_device;
    }

    /*Create proc directory. It will create a directory under "/proc" */
    parent = proc_mkdir("haha", NULL);

    if (parent == NULL)
    {
        pr_info("Error creating proc entry");
        goto r_device;
    }

    /*Creating Proc entry under "/proc/haha/" */
    proc_create("procfs_proc", 0666, parent, &proc_fops);

    pr_info("Device Driver Insert...Done!!!\n");

    gpio_request(GPIO_LED, "LED");
    gpio_direction_output(GPIO_LED, 0);

    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    return -1;
}

/*
** Module exit function
*/
static void __exit haha_driver_exit(void)
{
    /* Removes single proc entry */
    //remove_proc_entry("haha/haha_proc", parent);

    /* remove complete /proc/haha */
    proc_remove(parent);

    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&haha_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!!\n");
}

module_init(haha_driver_init);
module_exit(haha_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("Simple Linux device driver (procfs)");
MODULE_VERSION("1.6");