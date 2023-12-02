/***************************************************************************//**
*  \file       driver.c
*
*  \details    Simple GPIO driver explanation (GPIO Interrupt)
*
*  \author     EmbeTronicX
*
*  \Tested with Linux raspberrypi 5.4.51-v7l+
*
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/gpio.h>     //GPIO
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/jiffies.h>
/* Since debounce is not supported in Raspberry pi, I have addded this to disable 
** the false detection (multiple IRQ trigger for one interrupt).
** Many other hardware supports GPIO debounce, I don't want care about this even 
** if this has any overhead. Our intention is to explain the GPIO interrupt.
** If you want to disable this extra coding, you can comment the below macro.
** This has been taken from : https://raspberrypi.stackexchange.com/questions/8544/gpio-interrupt-debounce
**
** If you want to use Hardaware Debounce, then comment this EN_DEBOUNCE.
**
*/

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
 
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
 
 
/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, 
                char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, 
                const char *buf, size_t len, loff_t * off);
/******************************************************/

#define EN_DEBOUNCE

#include <linux/ktime.h>
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
//=================================================================================
//Interrupt handler for GPIO 25. This will be called whenever there is a raising edge detected. 
static irqreturn_t gpio_irq_handler(int irq,void *dev_id) 
{
  int value_of_25 = gpio_get_value(GPIO_26_IN);
  if(value_of_25 == 1)
  {
    start_time = ktime_get_ns();
    pr_info("HIGH\n");
    sprintf(uart_buff,"Interrupt occured\n\r");
    uart_send_str(uart_buff);
  }
  else
  {
    end_time = ktime_get_ns();
    sprintf(uart_buff,"Interrupt occured\n\r");
    uart_send_str(uart_buff);
    pr_info("LOW\n");  
    s64 diff_time = ktime_to_ns(ktime_sub(end_time, start_time));
    u64 diff_time_us = div_u64(diff_time, 1000);
    //sprintf(uart_buff,"up:diff_time_ms: %u ms\n\r", diff_time_us);
    sprintf(uart_buff,"Distance = %d cm \r\n", (int)diff_time_us * 17 / 1000);
    uart_send_str(uart_buff);
    pr_info("Distance = %d cm \r\n", (int)diff_time_us * 17 / 1000);
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
 
//File operation structure 
static struct file_operations fops =
{
  .owner          = THIS_MODULE,
  .read           = etx_read,
  .write          = etx_write,
  .open           = etx_open,
  .release        = etx_release,
};

/*
** This function will be called when we open the Device file
*/ 
static int etx_open(struct inode *inode, struct file *file)
{
  pr_info("Device File Opened...!!!\n");
  return 0;
}

/*
** This function will be called when we close the Device file
*/ 
static int etx_release(struct inode *inode, struct file *file)
{
  pr_info("Device File Closed...!!!\n");
  return 0;
}

/*
** This function will be called when we read the Device file
*/ 
static ssize_t etx_read(struct file *filp, 
                char __user *buf, size_t len, loff_t *off)
{
  uint8_t gpio_state = 0;
  //reading GPIO value
  gpio_state = gpio_get_value(GPIO_19_OUT);
  //write to user
  len = 1;
  if( copy_to_user(buf, &gpio_state, len) > 0) {
    pr_err("ERROR: Not all the bytes have been copied to user\n");
  }
  pr_info("Read function : GPIO_21 = %d \n", gpio_state);
  return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, 
                const char __user *buf, size_t len, loff_t *off)
{
  uint8_t rec_buf[10] = {0};
  if( copy_from_user( rec_buf, buf, len ) > 0) {
    pr_err("ERROR: Not all the bytes have been copied from user\n");
  }
  pr_info("Write Function : GPIO_21 Set = %c\n", rec_buf[0]);
  if (rec_buf[0]=='1') {
    //set the GPIO value to HIGH
    gpio_set_value(GPIO_19_OUT, 1);
  } else if (rec_buf[0]=='0') {
    //set the GPIO value to LOW
    gpio_set_value(GPIO_19_OUT, 0);
  } else {
    pr_err("Unknown command : Please provide either 1 or 0 \n");
  }
  return len;
}
/*
** Module Init function
*/ 
static int __init etx_driver_init(void)
{
  /*Allocating Major number*/
  if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
    pr_err("Cannot allocate major number\n");
    goto r_unreg;
  }
  pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
  /*Creating cdev structure*/
  cdev_init(&etx_cdev,&fops);
  /*Adding character device to the system*/
  if((cdev_add(&etx_cdev,dev,1)) < 0){
    pr_err("Cannot add the device to the system\n");
    goto r_del;
  }
  /*Creating struct class*/
  if(IS_ERR(dev_class = class_create(THIS_MODULE,"etx_class"))){
    pr_err("Cannot create the struct class\n");
    goto r_class;
  }
  /*Creating device*/
  if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){
    pr_err( "Cannot create the Device \n");
    goto r_device;
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
  pr_info("GPIO_irqNumber = %d\n", GPIO_irqNumber);
  if (request_irq(GPIO_irqNumber,             //IRQ number
                  (void *)gpio_irq_handler,   //IRQ handler
                  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,        //Handler will be called in raising edge
                  "etx_device",               //used to identify the device name using this IRQ
                  NULL)) {                    //device id for shared IRQ
    pr_err("my_device: cannot register IRQ ");
    goto r_gpio_in;
  }
  int i;
  kernel_timer_register();
  uart5_init();
  for(i=0;i<5;i++)
  {
      sprintf(uart_buff,"haha : %d \r\n", i);
      uart_send_str(uart_buff);
  }
  pr_info("Device Driver Insert...Done!!!\n");
  return 0;

r_gpio_in:
  gpio_free(GPIO_26_IN);
r_gpio_out:
  gpio_free(GPIO_19_OUT);
r_device:
  device_destroy(dev_class,dev);
r_class:
  class_destroy(dev_class);
r_del:
  cdev_del(&etx_cdev);
r_unreg:
  unregister_chrdev_region(dev,1);
  return -1;
}

/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
    del_timer(&timer);
    free_irq(GPIO_irqNumber,NULL);
    gpio_free(GPIO_26_IN);
    gpio_free(GPIO_19_OUT);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!\n");
}
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - GPIO Driver (GPIO Interrupt) ");
MODULE_VERSION("1.33");