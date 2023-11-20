#include <linux/kernel.h> // printk
#include <linux/init.h> //module_init() and module_exit()
#include <linux/module.h> // MODULE_LICENSE("GPL"); MODULE_AUTHOR(); MODULE_DESCRIPTION(""); MODULE_VERSION("");
#include <linux/io.h> // ioremap(), iounmap()
#include <linux/interrupt.h> //interrupt
#include <linux/gpio.h> //gpio
#include <linux/delay.h>//delay
#define MOD_MAJOR 207
#define MOD_NAME "driver-control"
// for BCM2711 GPIO Physical address : 0x7E200000
#define GPIO_BASE 0xFE200000 
#define BLOCK_SIZES 4096

#define GPIO_LED 18 // BCD_GPIO #18
#define GPIO_BTN 17 // BCM_GPIO #17

#define GPFSEL1 (0x04/4)
#define GPSET0 (0x1C/4)
#define GPCLR0 (0x28/4)

volatile unsigned int* gpio_addr;
static int btn_irq;
static int count = 0;
static int     __init IOCTL_Interrupt_Button_LED_init(void); //initailization 
static void    __exit IOCTL_Interrupt_Button_LED_exit(void); //exit

int myapi_open(struct inode* minode, struct file* mfile) {
    printk("Kernel Module Open(): %s\n", MOD_NAME);
    return 0;
}

int myapi_release(struct inode* minode, struct file* mfile) {
    printk("Kernel Module close(): %s\n", MOD_NAME);
    return 0;
}

ssize_t myapi_write(struct file* inode, const char* gdata, size_t length, loff_t* off_what)
{
    unsigned char c;
    printk("Kernel Module write(): %s\n", MOD_NAME);
    get_user(c, gdata);
    gpio_set_value(GPIO_LED, ((c == '0') ? 0 : 1));
    return length;
}

static ssize_t myapi_read(struct file* filp,
    char __user* buf, size_t len, loff_t* off)
{
    printk("Kernel Module Read(): %s\n", MOD_NAME);
    *(gpio_addr + GPSET0) |= 1 << (GPIO_LED);
    mdelay(2000);
    *(gpio_addr + GPCLR0) |= 1 << (GPIO_LED);
    return 0; 
}


long int myapi_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
    case 0:
        printk("LED ON = %02x..\n", (unsigned char)arg);
        gpio_set_value(GPIO_LED, 1);
        break;
    case 1:
        printk("LED OFF = %02x..\n", (unsigned char)arg);
        gpio_set_value(GPIO_LED, 0);
        break;
    default:
        printk("ioctl command mismatch ERROR! \n");
    }
    return 0;
}

static struct file_operations myfops = {
.write = myapi_write, .read = myapi_read, .open = myapi_open, .release = myapi_release, .unlocked_ioctl = myapi_ioctl
};

//Intterupt service routine
static irqreturn_t btn_interrupt(int irq, void* dev_id) {
    printk("interrupt occurred(%d) !! \n", count++);
    if (count % 2 == 0)
    {
        gpio_set_value(GPIO_LED, 1);
        printk("led on\n");
    }
    else
    {
        gpio_set_value(GPIO_LED, 0);
        printk("led off\n");
    }
    return 0;
}


static int __init IOCTL_Interrupt_Button_LED_init(void)
{
    int irq;
    //<--- IO    --->  
    gpio_addr = ioremap(GPIO_BASE, BLOCK_SIZES);
    //GPIO clear
    *(gpio_addr + GPFSEL1) &= ~(0x07 << 24);
    //output setting
    *(gpio_addr + GPFSEL1) |= (0x01 << 24);
    //<---charter driver initalization --->
    register_chrdev(MOD_MAJOR, MOD_NAME, &myfops);
    //<---GPIO Initalization --->
    gpio_request(GPIO_LED, "LED");
    gpio_direction_output(GPIO_LED, 0);
    gpio_request(GPIO_BTN, "SWITCH");
    gpio_direction_input(GPIO_BTN);
    //<--- IRQ --> 
    btn_irq = gpio_to_irq(GPIO_BTN);
    irq = request_irq(btn_irq, &btn_interrupt, IRQF_TRIGGER_FALLING, "SWITCH", NULL);
    //can be checked at dmesg
    printk("insert module sucessful\n");
    return 0;
}

static void __exit IOCTL_Interrupt_Button_LED_exit(void)  //calls on sudo rmmod my_drvice_driver.ko 
{
    iounmap(gpio_addr);
    unregister_chrdev(MOD_MAJOR, MOD_NAME);
    gpio_free(GPIO_LED);
    pr_info("Device Driver Remove...Done!!!\n");
   
}
module_init(IOCTL_Interrupt_Button_LED_init);
module_exit(IOCTL_Interrupt_Button_LED_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Justin Kim jiwook021@gmail.com");
MODULE_DESCRIPTION("IOCTL_Interrupt_Button_LED");
MODULE_VERSION("1.4");