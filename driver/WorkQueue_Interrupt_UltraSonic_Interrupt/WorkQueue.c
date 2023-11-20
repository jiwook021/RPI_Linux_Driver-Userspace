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
#include <linux/delay.h>
#include <linux/workqueue.h>            // Required for workqueues
#include <linux/err.h>
#include <linux/gpio.h>

#define MOD_MAJOR 201
#define MOD_NAME "WorkQueue"

static int btn_irq;

#include <linux/ktime.h>
s64 diff_time;
u64 diff_time_us;
//Ultrasonic sensor
#define GPIO_19_OUT (19) //trigger
#define GPIO_26_IN  (26) //echo

//GPIO_26_IN value toggle
unsigned int led_toggle = 0; 
//This used for storing the IRQ number for the GPIO
unsigned int GPIO_irqNumber;

u64 start_time, end_time ;
//USB
#define UART5_BASE 0xFE201A00 // 0xFE201A00 : Virtual Address
#define BLOCK_SIZE 4096
volatile unsigned int *uart5_addr;
volatile unsigned int *uart5_dr;
volatile unsigned int *uart5_fr;
volatile unsigned int *uart5_ibrd;
volatile unsigned int *uart5_fbrd;

struct timer_list timer;

char uart_buff[100];

void uart5_init(void)
{
    uart5_addr = ioremap(UART5_BASE, BLOCK_SIZE);
    uart5_dr = uart5_addr + 0x00;
    uart5_fr = uart5_addr + 0x18/4;
    uart5_ibrd = uart5_addr + 0x24/4;
    uart5_fbrd = uart5_addr + 0x28/4;        
    //------------------------------------
    // FBRD is a 6 bit number (0-63) to represent the fractional divisor.
    //-----------------
    // The uart clock is 48MHz, and we are going to use a fixed, 115,200 baud rate. So...
    // BAUDDIV = 48,000,000 / (16 * 115,200) = 26.041
    // IBRD = floor(26.041) =  26
    // FBRD = 0.041* 64   = 3(2.624)
    //-----------------
    /* default uart clock : 48 MHz, Baudrate : 115200 */
    *uart5_ibrd = 26;
    *uart5_fbrd = 3;
    //------------------------------------
}

void uart_send_char(char data)
{
    while(*uart5_fr & (0x01 << 5));  // 5 : TXFE(if fifo enabled)
    *uart5_dr = data;    
}

void uart_send_str(char *str)
{
    int i;
    int str_len = strlen(str);
    for(i=0;i<str_len;i++)
    {
        uart_send_char(str[i]);
    }
}
//Interrupt handler for GPIO 25. This will be called whenever there is a raising edge detected. 
static irqreturn_t gpio_irq_handler(int irq,void *dev_id) 
{
    int value_of_25 = gpio_get_value(GPIO_26_IN);
    if(value_of_25 == 1)
    {
        start_time = ktime_get_ns();
        pr_info("HIGH\n");
        sprintf(uart_buff,"Interrupt occured: button Down\n\r");
        uart_send_str(uart_buff);
    }
    else
    {
        end_time = ktime_get_ns();
        sprintf(uart_buff,"Interrupt occured: button UP\n\r");
        uart_send_str(uart_buff);
        pr_info("LOW\n");
        diff_time = ktime_to_ns(ktime_sub(end_time, start_time));
    }
    return IRQ_HANDLED;
}

int count = 0;
void kernel_timer_func(struct timer_list *);
void kernel_timer_func(struct timer_list *timer)
{
    // printk("haha : %d \n", count++);
    gpio_set_value(GPIO_19_OUT, 1); 
    ndelay(10000);
    gpio_set_value(GPIO_19_OUT, 0); 
    // mod_timer(timer, get_jiffies_64() + DELAY_TIME_SEC);
    mod_timer(timer, get_jiffies_64() + 10);
}

static void kernel_timer_register(void)
{
    timer_setup(&timer, kernel_timer_func, 0);
    timer.expires = get_jiffies_64() + 10;
    add_timer(&timer);
}

void workqueue_fn(struct work_struct* work);

/*Creating work by Static Method */
DECLARE_WORK(workqueue, workqueue_fn);

/*Workqueue Function*/
void workqueue_fn(struct work_struct* work)
{
    diff_time_us = div_u64(diff_time, 1000);
    //sprintf(uart_buff,"up:diff_time_ms: %u ms\n\r", diff_time_us);
    sprintf(uart_buff,"Distance = %d cm \r\n", (int)diff_time_us * 17 / 1000);
    uart_send_str(uart_buff);
    pr_info("Distance = %d cm \r\n", (int)diff_time_us * 17 / 1000);
    printk(KERN_INFO "Executing Workqueue Function\n");
}
volatile int WorkQueue_value = 0;
dev_t dev = 0;
static struct class* dev_class;
static struct cdev WorkQueue_cdev;
struct kobject* kobj_ref;
/*
** Function Prototypes
*/
static int __init WorkQueue_driver_init(void);
static void __exit WorkQueue_driver_exit(void);

/*************** Driver Fuctions **********************/
static int WorkQueue_open(struct inode* inode, struct file* file);
static int WorkQueue_release(struct inode* inode, struct file* file);
static ssize_t WorkQueue_read(struct file* filp,
    char __user* buf, size_t len, loff_t* off);
static ssize_t WorkQueue_write(struct file* filp,
    const char* buf, size_t len, loff_t* off);

/*************** Sysfs Fuctions **********************/
static ssize_t sysfs_show(struct kobject* kobj,
    struct kobj_attribute* attr, char* buf);
static ssize_t sysfs_store(struct kobject* kobj,
    struct kobj_attribute* attr, const char* buf, size_t count);

struct kobj_attribute WorkQueue_attr = __ATTR(WorkQueue_value, 0660, sysfs_show, sysfs_store);
/*
** File operation sturcture
*/
static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .read = WorkQueue_read,
    .write = WorkQueue_write,
    .open = WorkQueue_open,
    .release = WorkQueue_release,
};
/*
** This function will be called when we read the sysfs file
*/
static ssize_t sysfs_show(struct kobject* kobj,
    struct kobj_attribute* attr, char* buf)
{
    printk(KERN_INFO "Sysfs - Read!!!\n");
    return sprintf(buf, "%d", WorkQueue_value);
}
/*
** This function will be called when we write the sysfsfs file
*/
static ssize_t sysfs_store(struct kobject* kobj,
    struct kobj_attribute* attr, const char* buf, size_t count)
{
    printk(KERN_INFO "Sysfs - Write!!!\n");
    sscanf(buf, "%d", &WorkQueue_value);
    return count;
}
/*
** This function will be called when we open the Device file
*/
static int WorkQueue_open(struct inode* inode, struct file* file)
{
    printk(KERN_INFO "Device File Opened...!!!\n");
    return 0;
}
/*
** This function will be called when we close the Device file
*/
static int WorkQueue_release(struct inode* inode, struct file* file)
{
    printk(KERN_INFO "Device File Closed...!!!\n");
    return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t WorkQueue_read(struct file* filp,
    char __user* buf, size_t len, loff_t* off)
{
    printk(KERN_INFO "Read function\n");
    //asm("int $0x3B");  // Corresponding to irq 11
    return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t WorkQueue_write(struct file* filp,
    const char __user* buf, size_t len, loff_t* off)
{
    printk(KERN_INFO "Write Function\n");
    return len;
}

/*
** Module Init function
*/
static int __init WorkQueue_driver_init(void)
{
    int irq; 
    /*Allocating Major number*/
    if ((alloc_chrdev_region(&dev, 2, 1, "WorkQueue_Dev")) < 0) {
        printk(KERN_INFO "Cannot allocate major number\n");
        return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));
    /*Creating cdev structure*/
    cdev_init(&WorkQueue_cdev, &fops);
    /*Adding character device to the system*/
    if ((cdev_add(&WorkQueue_cdev, dev, 1)) < 0) {
        printk(KERN_INFO "Cannot add the device to the system\n");
        goto r_class;
    }
    /*Creating struct class*/
    if (IS_ERR(dev_class = class_create(THIS_MODULE, "WorkQueue_class"))) {
        printk(KERN_INFO "Cannot create the struct class\n");
        goto r_class;
    }
    /*Creating device*/
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "WorkQueue_device"))) {
        printk(KERN_INFO "Cannot create the Device 1\n");
        goto r_device;
    }
    /*Creating a directory in /sys/kernel/ */
    kobj_ref = kobject_create_and_add("WorkQueue_sysfs", kernel_kobj);
    /*Creating sysfs file for WorkQueue_value*/
    if (sysfs_create_file(kobj_ref, &WorkQueue_attr.attr)) {
        printk(KERN_INFO"Cannot create sysfs file......\n");
        goto r_sysfs;
    }
    //Output GPIO configuration
    //Checking the GPIO is valid or not
    if(gpio_is_valid(GPIO_19_OUT) == false){
        pr_err("GPIO %d is not valid\n", GPIO_19_OUT);
        goto r_device;
    }
    //Requesting the GPIO
    if(gpio_request(GPIO_19_OUT,"GPIO_19_OUT") < 0){
        pr_err("ERROR: GPIO %d request\n", GPIO_19_OUT);
        goto r_gpio_out;
    } 
    //configure the GPIO as output
    gpio_direction_output(GPIO_19_OUT, 0);
    
    //Input GPIO configuratioin
    //Checking the GPIO is valid or not
    if(gpio_is_valid(GPIO_26_IN) == false){
        pr_err("GPIO %d is not valid\n", GPIO_26_IN);
        goto r_gpio_in;
    }
    
    //Requesting the GPIO
    if(gpio_request(GPIO_26_IN,"GPIO_26_IN") < 0){
        pr_err("ERROR: GPIO %d request\n", GPIO_26_IN);
        goto r_gpio_in;
    }
    
    //configure the GPIO as input
    gpio_direction_input(GPIO_26_IN);
    
    /*
    ** I have commented the below few lines, as gpio_set_debounce is not supported 
    ** in the Raspberry pi. So we are using EN_DEBOUNCE to handle this in this driver.
    */ 
    //Get the IRQ number for our GPIO
    GPIO_irqNumber = gpio_to_irq(GPIO_26_IN);

    irq = request_irq( GPIO_irqNumber, gpio_irq_handler, IRQF_TRIGGER_FALLING, "SWITCH", NULL);

   /* if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "WorkQueue_device", (void*)(irq_handler))) {
        printk(KERN_INFO "my_device: cannot register IRQ ");
        goto irq;
    }*/
     int i;
    kernel_timer_register();
    uart5_init();
    for(i=0;i<5;i++)
    {
        sprintf(uart_buff,"haha : %d \r\n", i);
        uart_send_str(uart_buff);
    }
    pr_info("Device Driver Insert...Done!!!\n");
    printk(KERN_INFO "Device Driver Insert...Done!!!\n");
    return 0;
r_gpio_in:
    gpio_free(GPIO_26_IN);
r_gpio_out:
    gpio_free(GPIO_19_OUT);
irq:
    free_irq(btn_irq, (void*)(gpio_irq_handler));
r_sysfs:
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &WorkQueue_attr.attr);
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    cdev_del(&WorkQueue_cdev);
    return -1;
}
/*
** Module exit function
*/
static void __exit WorkQueue_driver_exit(void)
{
    del_timer(&timer);
    gpio_free(GPIO_26_IN);
    gpio_free(GPIO_19_OUT);
    free_irq(btn_irq, (void*)(gpio_irq_handler));
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &WorkQueue_attr.attr);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&WorkQueue_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}

module_init(WorkQueue_driver_init);
module_exit(WorkQueue_driver_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Linux device driver (Global Workqueue - Static method)");
MODULE_AUTHOR("Justin Kim jiwook021@gmail.com");