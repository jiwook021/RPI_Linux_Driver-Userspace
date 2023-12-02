#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/err.h>
#include <linux/gpio.h> //gpio
//Timer Variable
#define TIMEOUT_SERVO_SEC   ( 16000L )      //16 us in nano seconds
#define TIMEOUT_SEC    ( 0 )                
static struct hrtimer LED_hr_timer;
static struct hrtimer Servo_hr_timer;
static unsigned int count = 0;
//Timer Variable
#define TIMEOUT_LED_100US   ( 100000L )      //100 us in nano seconds
#define TIMEOUT_SEC    ( 0 )         
//LED
#define GPIO_LED 18 // BCD_GPIO #18

//SERVO
#define GPIO_SERVO 23 

dev_t dev = 0;
static struct class *dev_class;
static struct cdev sample_cdev;
 
static int __init sample_driver_init(void);
static void __exit sample_driver_exit(void);
/*************** Driver functions **********************/
static int sample_open(struct inode *inode, struct file *file);
static int sample_release(struct inode *inode, struct file *file);
static ssize_t sample_read(struct file *filp, 
                                char __user *buf, size_t len,loff_t * off);
static ssize_t sample_write(struct file *filp, 
                                const char *buf, size_t len, loff_t * off);
/******************************************************/
//File operation structure  
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = sample_read,
        .write          = sample_write,
        .open           = sample_open,
        .release        = sample_release,
};


int LED_flag = 1; 
int led_duty = 1;
//Timer Callback function. This will be called when timer expires
enum hrtimer_restart led_timer_callback(struct hrtimer *timer)
{
     /* do your timer stuff here */
    // pr_info("Timer Callback function Called [%d]\n",count++);
    if (LED_flag)
    {
        gpio_set_value(GPIO_LED, 1); 
        LED_flag = 0;
        hrtimer_forward_now(timer,ktime_set(TIMEOUT_SEC, TIMEOUT_LED_100US*(led_duty)));
    }
    else
    {
        gpio_set_value(GPIO_LED, 0);
        hrtimer_forward_now(timer,ktime_set(TIMEOUT_SEC, TIMEOUT_LED_100US*(99 - led_duty)));
        LED_flag = 1;
    }
    return HRTIMER_RESTART;
}

 
int servo_flag = 1; 
int servo_duty = 44;
//Timer Callback function. This will be called when timer expires
enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
    //pr_info("Timer Callback function Called [%d]\n",count++);
    if (servo_flag)
    {
        gpio_set_value(GPIO_SERVO, 1); 
        hrtimer_forward_now(timer,ktime_set(TIMEOUT_SEC, TIMEOUT_SERVO_SEC*(servo_duty)));
        servo_flag = 0;
    }
    else
    {
        gpio_set_value(GPIO_SERVO, 0);
        hrtimer_forward_now(timer,ktime_set(TIMEOUT_SEC, TIMEOUT_SERVO_SEC*(1250 - servo_duty)));
        servo_flag = 1;
    }
    return HRTIMER_RESTART;
}

struct timer_list led_timer;
struct timer_list servo_timer;

void servo_kernel_timer_func(struct timer_list *);
void servo_kernel_timer_func(struct timer_list *timer)
{
    servo_duty++;
    if(servo_duty==144)
        servo_duty = 44;
    mod_timer(timer, get_jiffies_64() + 2);
}

void kernel_timer_led_func(struct timer_list *);
void kernel_timer_led_func(struct timer_list *timer)
{
    led_duty++;
    if(led_duty==99)
        led_duty = 1;
    mod_timer(timer, get_jiffies_64() + 1);
}
static void kernel_timer_led_register(void)
{
    timer_setup(&led_timer, kernel_timer_led_func, 0);
    led_timer.expires = get_jiffies_64() + 10;
    add_timer(&led_timer);
}

static void kernel_timer_register(void)
{
    timer_setup(&servo_timer, servo_kernel_timer_func, 0);
    servo_timer.expires = get_jiffies_64() + 2;
    add_timer(&servo_timer);
}
/*
** This function will be called when we open the Device file
*/
static int sample_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}
/*
** This function will be called when we close the Device file
*/ 
static int sample_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t sample_read(struct file *filp, 
                                char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read Function\n");
    return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t sample_write(struct file *filp, 
                                const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write function\n");
    return len;
}
/*
** Module Init function
*/ 
static int __init sample_driver_init(void)
{
     ktime_t ktime;
     ktime_t led_ktime;
    
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "sample_Dev")) <0){
            pr_err("Cannot allocate major number\n");
            return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
    /*Creating cdev structure*/
    cdev_init(&sample_cdev,&fops);
 
    /*Adding character device to the system*/
    if((cdev_add(&sample_cdev,dev,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }
 
    /*Creating struct class*/
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"sample_class"))){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }
 
    /*Creating device*/
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"sample_device"))){
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }
    
    ktime = ktime_set(TIMEOUT_SEC, TIMEOUT_SERVO_SEC);
    hrtimer_init(&Servo_hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    Servo_hr_timer.function = &timer_callback;
    hrtimer_start( &Servo_hr_timer, ktime, HRTIMER_MODE_REL);
 
    //led initaliztion
    gpio_request(GPIO_SERVO, "SERVO");
    gpio_direction_output(GPIO_SERVO, 0);
    //timer for changing brightness
    kernel_timer_register();


    led_ktime = ktime_set(TIMEOUT_SEC, TIMEOUT_LED_100US);
    hrtimer_init(&LED_hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    //LED_timer
    LED_hr_timer.function = &led_timer_callback;
    hrtimer_start( &LED_hr_timer, led_ktime, HRTIMER_MODE_REL);
    //led initaliztion
    gpio_request(GPIO_LED, "LED");
    gpio_direction_output(GPIO_LED, 0);
    //timer for changing brightness
    kernel_timer_led_register();
    pr_info("Device Driver Insert...Done!!!\n");
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
static void __exit sample_driver_exit(void)
{
    //stop the timer
    hrtimer_cancel(&LED_hr_timer);
    hrtimer_cancel(&Servo_hr_timer);
    device_destroy(dev_class,dev); 
    class_destroy(dev_class);
    cdev_del(&sample_cdev);
    unregister_chrdev_region(dev, 1);
    del_timer(&servo_timer);
    del_timer(&led_timer);
    gpio_free(GPIO_SERVO);
    gpio_free(GPIO_LED);
    pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(sample_driver_init);
module_exit(sample_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - High Resolution Timer");
MODULE_VERSION("1.22");