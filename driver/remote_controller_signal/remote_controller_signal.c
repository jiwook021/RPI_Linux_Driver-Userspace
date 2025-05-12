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
#include <linux/ktime.h>

static long sample_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


#define UART5_BASE 0xFE201A00 // 0xFE201A00 : Virtual Address
#define BLOCK_SIZE 4096
volatile unsigned int *uart5_addr;
volatile unsigned int *uart5_dr;
volatile unsigned int *uart5_fr;
volatile unsigned int *uart5_ibrd;
volatile unsigned int *uart5_fbrd;

char uart_buff[100];
//12 rx 13 tx


#define EN_DEBOUNCE

#ifdef EN_DEBOUNCE
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
#include <linux/wait.h>                 // Required for the wait queues
#include <linux/kthread.h>
#define SIGETX 44 
#define REG_CURRENT_TASK _IOW('a','a',int32_t*)
static struct task_struct *task = NULL;

static struct task_struct *wait_thread;
wait_queue_head_t wait_queue_etx;
int wait_queue_flag = 0;

int output;
int i;
u64 difftime;
char hex;
u64 start_time, end_time ;
u64 capture_time_array[35];
u64 diff_time_us[33]; 
int bit_array[32];
int counter = 0;
static void signalfunc(int output);

char convertBinaryToHex(int binary[32]) {
    int hexValue = 0;
    int i;
    for (i = 16; i < 24; i++) {
        hexValue += binary[i] << (7 - (i-16));
    }

    if (hexValue < 10) {
        return '0' + hexValue;
    } else {
        return 'A' + hexValue - 10;
    }
}

static void remote_control_function(void)
{
  //--------------------------------------
  // for(i=0;i<34;i++)
  // {
  //   sprintf(uart_buff,"time: %u ns\n\r", capture_time_array[i]);
  //   uart_send_str(uart_buff);
  // }
  //--------------------------------------
  // for(i=0;i<33;i++)
  // {
  //   difftime = capture_time_array[i+1]- capture_time_array[i];
  //   diff_time_us[i] = div_u64(difftime, 1000);
  // }
  //--------------------------------------
  // for(i=0;i<33;i++)
  // {
  //   sprintf(uart_buff,"diff_time: %u us\n\r", diff_time_us[i]);
  //   uart_send_str(uart_buff);
  // }
  // sprintf(uart_buff,"\n\r");
  // uart_send_str(uart_buff);
  //--------------------------------------
  for(i=0;i<32;i++)
  {
    if(diff_time_us[i] > 1000 && diff_time_us[i] < 1500)
      bit_array[i] = 0;
    else if(diff_time_us[i] > 2000 && diff_time_us[i] < 2500)
      bit_array[i] = 1;       
  }
  // //--------------------------------------
  // for(i=0;i<32;i++)
  // {
  //   // sprintf(uart_buff,"bit array[%d]: %d \n\r", i, bit_array[i]);
  //   // uart_send_str(uart_buff);
  //   printk("bit array[%d]: %d \n\r", i, bit_array[i]);
  // }
  //--------------------------------------
  // for(i=16;i<24;i++)
  // {
  //   // sprintf(uart_buff,"bit array[%d]: %d \n\r", i, bit_array[i]);
  //   // uart_send_str(uart_buff);
  //   printk("bit array[%d]: %d \n\r", i, bit_array[i]);
  // }
  hex = convertBinaryToHex(bit_array);
  printk("hex: %d \n\r", hex);
  // sprintf(uart_buff,"hex: %d \n\r", hex);
  // uart_send_str(uart_buff);
  switch (hex) {
    case 235:
        output =0; 
        break;
    case 207:
        output =1; 
        break;
    case 195:
        output =2; 
        break;
    case 244:
        output =3; 
        break;

    case 191:
        output =4; 
        break;
    case 211:
        output =5; 
        break;
    case 228:
        output =6; 
        break;
    case 216:
        output =7; 
        break;
    case 220:
        output =8; 
        break;
    case 224:
        output =9; 
        break;
    default:
        printk("default\n");
  }
  printk("%d\n", output);
  // sprintf(uart_buff,"output: %d \n\r", output);
  // uart_send_str(uart_buff);

  // sprintf(uart_buff,"\n\r");
  // uart_send_str(uart_buff);  
  signalfunc(output);
  printk("\n\r");
}

static void signalfunc(int output)
{
  struct kernel_siginfo info;
  printk(KERN_INFO "Shared IRQ: Interrupt Occurred");
  //Sending signal to app
  memset(&info, 0, sizeof(struct kernel_siginfo));//kernel_siginfo
  info.si_signo = SIGETX;
  info.si_code = SI_QUEUE;
  info.si_int = output;
  if (task != NULL) {
    printk(KERN_INFO "Sending signal to app\n");
    if(send_sig_info(SIGETX, &info, task) < 0) {
      printk(KERN_INFO "Unable to send signal\n");
    }
  }
}

static int wait_function(void *unused)
{      
  while(1) {
          pr_info("Waiting For Event...\n");
          wait_event_interruptible(wait_queue_etx, wait_queue_flag != 0 );
          if(wait_queue_flag == 2) {
                  pr_info("Event Came From Exit Function\n");
                  return 0;
          }
          pr_info("Event Came From Read Function\n");
          remote_control_function();
          wait_queue_flag = 0;
  }
  return 0;
}

//===================================================================================


static int signum = 0;

//====================================================================================
#include <linux/jiffies.h>

extern unsigned long volatile jiffies;
unsigned long old_jiffie = 0;
#endif

//LED is connected to this GPIO
#define GPIO_21_OUT (21)

//LED is connected to this GPIO
#define GPIO_25_IN  (25)

//GPIO_25_IN value toggle
unsigned int led_toggle = 0; 

//This used for storing the IRQ number for the GPIO
unsigned int GPIO_irqNumber;


 
//Interrupt handler for GPIO 25. This will be called whenever there is a raising edge detected. 
static irqreturn_t gpio_irq_handler(int irq,void *dev_id) 
{ 
  
  u64 capture_time = ktime_to_ns(ktime_get());
  capture_time_array[counter] = capture_time;
  
  if(counter>0)
  {  
    difftime = capture_time_array[counter]- capture_time_array[counter-1];
    diff_time_us[counter-1] = div_u64(difftime, 1000);
    if(diff_time_us[counter-1]> 13000 && diff_time_us[counter-1]< 14000)
    {
      // sprintf(uart_buff,"LEAD CODE\n\r");
      // uart_send_str(uart_buff);
      // printk("LEAD CODE\n\r");
      counter = 1;
    }
    else if(diff_time_us[counter-1]> 11000 && diff_time_us[counter-1]< 12000)
    {
      // sprintf(uart_buff,"REPEATE %d\n\r", output);
      // uart_send_str(uart_buff);
      // printk("REPEATE\n\r");
      printk("%d\n", output);
      signalfunc(output);
      counter = -1;
    }
  }

  counter++;
  if(counter == 34)
  {
    wait_queue_flag = 1;
    wake_up_interruptible(&wait_queue_etx);
    counter = 0;
    // //--------------------------------------
    // // for(i=0;i<34;i++)
    // // {
    // //   sprintf(uart_buff,"time: %u ns\n\r", capture_time_array[i]);
    // //   uart_send_str(uart_buff);
    // // }
    // //--------------------------------------
    // // for(i=0;i<33;i++)
    // // {
    // //   difftime = capture_time_array[i+1]- capture_time_array[i];
    // //   diff_time_us[i] = div_u64(difftime, 1000);
    // // }
    // //--------------------------------------
    // // for(i=0;i<33;i++)
    // // {
    // //   sprintf(uart_buff,"diff_time: %u us\n\r", diff_time_us[i]);
    // //   uart_send_str(uart_buff);
    // // }
    // // sprintf(uart_buff,"\n\r");
    // // uart_send_str(uart_buff);
    // //--------------------------------------
    // for(i=0;i<32;i++)
    // {
    //   if(diff_time_us[i] > 1000 && diff_time_us[i] < 1500)
    //     bit_array[i] = 0;
    //   else if(diff_time_us[i] > 2000 && diff_time_us[i] < 2500)
    //     bit_array[i] = 1;       
    // }
    // // //--------------------------------------
    // // for(i=0;i<32;i++)
    // // {
    // //   // sprintf(uart_buff,"bit array[%d]: %d \n\r", i, bit_array[i]);
    // //   // uart_send_str(uart_buff);
    // //   printk("bit array[%d]: %d \n\r", i, bit_array[i]);
    // // }
    // //--------------------------------------
    // // for(i=16;i<24;i++)
    // // {
    // //   // sprintf(uart_buff,"bit array[%d]: %d \n\r", i, bit_array[i]);
    // //   // uart_send_str(uart_buff);
    // //   printk("bit array[%d]: %d \n\r", i, bit_array[i]);
    // // }
    // hex = convertBinaryToHex(bit_array);
    // printk("hex: %d \n\r", hex);
    // // sprintf(uart_buff,"hex: %d \n\r", hex);
    // // uart_send_str(uart_buff);


    // switch (hex) {
    //     case 235:
    //         output =0; 
    //         break;
    //     case 207:
    //         output =1; 
    //         break;
    //     case 195:
    //         output =2; 
    //         break;
    //     case 244:
    //         output =3; 
    //         break;

    //     case 191:
    //         output =4; 
    //         break;
    //     case 211:
    //         output =5; 
    //         break;
    //     case 228:
    //         output =6; 
    //         break;
    //     case 216:
    //         output =7; 
    //         break;
    //     case 220:
    //         output =8; 
    //         break;
    //     case 224:
    //         output =9; 
    //         break;
    //     default:
    //         printk("default\n");
    // }
    // printk("%d\n", output);
    // // sprintf(uart_buff,"output: %d \n\r", output);
    // // uart_send_str(uart_buff);

    // // sprintf(uart_buff,"\n\r");
    // // uart_send_str(uart_buff);  
    
    // printk("\n\r");
    
  }
  return IRQ_HANDLED;
}
 
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

//File operation structure 
static struct file_operations fops =
{
  .owner          = THIS_MODULE,
  .read           = etx_read,
  .write          = etx_write,
  .open           = etx_open,
  .unlocked_ioctl = sample_ioctl,
  .release        = etx_release,
};
static long sample_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if (cmd == REG_CURRENT_TASK) {
        printk(KERN_INFO "REG_CURRENT_TASK\n");
        task = get_current();
        signum = SIGETX;
    }
    return 0;
}
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
  gpio_state = gpio_get_value(GPIO_21_OUT);
  
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
    gpio_set_value(GPIO_21_OUT, 1);
  } else if (rec_buf[0]=='0') {
    //set the GPIO value to LOW
    gpio_set_value(GPIO_21_OUT, 0);
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
  if(gpio_is_valid(GPIO_21_OUT) == false){
    pr_err("GPIO %d is not valid\n", GPIO_21_OUT);
    goto r_device;
  }
  
  //Requesting the GPIO
  if(gpio_request(GPIO_21_OUT,"GPIO_21_OUT") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_21_OUT);
    goto r_gpio_out;
  }
  
  //configure the GPIO as output
  gpio_direction_output(GPIO_21_OUT, 0);
  
  //Input GPIO configuratioin
  //Checking the GPIO is valid or not
  if(gpio_is_valid(GPIO_25_IN) == false){
    pr_err("GPIO %d is not valid\n", GPIO_25_IN);
    goto r_gpio_in;
  }
  
  //Requesting the GPIO
  if(gpio_request(GPIO_25_IN,"GPIO_25_IN") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_25_IN);
    goto r_gpio_in;
  }
  
  //configure the GPIO as input
  gpio_direction_input(GPIO_25_IN);
  
  /*
  ** I have commented the below few lines, as gpio_set_debounce is not supported 
  ** in the Raspberry pi. So we are using EN_DEBOUNCE to handle this in this driver.
  */ 
#ifndef EN_DEBOUNCE
  //Debounce the button with a delay of 200ms
  if(gpio_set_debounce(GPIO_25_IN, 200) < 0){
    pr_err("ERROR: gpio_set_debounce - %d\n", GPIO_25_IN);
    //goto r_gpio_in;
  }
#endif
  
  //Get the IRQ number for our GPIO
  GPIO_irqNumber = gpio_to_irq(GPIO_25_IN);
  pr_info("GPIO_irqNumber = %d\n", GPIO_irqNumber);
  
  if (request_irq(GPIO_irqNumber,             //IRQ number
                  (void *)gpio_irq_handler,   //IRQ handler
                  IRQF_TRIGGER_FALLING,        //Handler will be called in raising edge
                  "etx_device",               //used to identify the device name using this IRQ
                  NULL)) {                    //device id for shared IRQ
    pr_err("my_device: cannot register IRQ ");
    goto r_gpio_in;
  }
  int i;
//   uart5_init();
//   for(i=0;i<5;i++)
// {
//     sprintf(uart_buff,"haha : %d \r\n", i);
//     uart_send_str(uart_buff);
// }

  //===========================================
//Initialize wait queue
  init_waitqueue_head(&wait_queue_etx); 
  //Create the kernel thread with name 'mythread'
  wait_thread = kthread_create(wait_function, NULL, "WaitThread");
  if (wait_thread) {
    pr_info("Thread Created successfully\n");
    wake_up_process(wait_thread);
  } else
  pr_info("Thread creation failed\n");
  //============================================
 
  pr_info("Device Driver Insert...Done!!!\n");
  return 0;

r_gpio_in:
  gpio_free(GPIO_25_IN);
r_gpio_out:
  gpio_free(GPIO_21_OUT);
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
  free_irq(GPIO_irqNumber,NULL);
  gpio_free(GPIO_25_IN);
  gpio_free(GPIO_21_OUT);
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