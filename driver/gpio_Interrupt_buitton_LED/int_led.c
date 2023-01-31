#include <linux/interrupt.h> // *)!!
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/io.h>

#include <linux/gpio.h> // *)!!
#include <asm/uaccess.h>
#define MOD_MAJOR 203
#define MOD_NAME "int_led"

#define GPIO_BTN 17 // BCM_GPIO #17
#define GPIO_LED 18 // BCM_GPIO #18

// sudo mknod /dev/int_led c 203 0


static int btn_irq;
static char msg[40] = {0};

int count = 0;

// interrupt handler
static irqreturn_t btn_interrupt(int irq, void *dev_id) {
    char temp[40] = "GPIO Switch was Pushed!";
    printk("interrupt occurred(%d) !! \n", count++);
    strcpy(msg, temp);
    if (count%2 ==0)
    {
        gpio_set_value(GPIO_LED, (1));
         printk("led on\n");
    }
    else 
    {
        gpio_set_value(GPIO_LED, (0));
        printk("led off\n");
    }
    return 0;
}

static ssize_t gpiobtn_read(struct file *filp, char *buf, size_t count, loff_t *l) {
    int result;
    
    result = copy_to_user(buf, &msg, sizeof(msg));
    memset(msg, 0, 40);

    return result;
}

static int gpiobtn_open(struct inode *inode, struct file *filp) {
    printk("Kernel Module Open(): %s\n", MOD_NAME);
    
    return 0;
}

static int gpiobtn_release(struct inode *inode, struct file *filp) {
    printk("Kernel Module close(): %s\n", MOD_NAME);
    disable_irq(GPIO_BTN);
    
    return 0;
}

static struct file_operations gpiobtn_fops = {
.read = gpiobtn_read,
.open = gpiobtn_open,
.release = gpiobtn_release,
};

int gpiobtn_init(void) {
    int result, irq;
    
    result = register_chrdev(MOD_MAJOR, MOD_NAME , &gpiobtn_fops);
    if(result < 0) {
        printk(KERN_WARNING"Can't get major %d\n", MOD_MAJOR);
    }
    
    gpio_request(GPIO_BTN, "SWITCH");
    gpio_direction_input(GPIO_BTN);

    gpio_request(GPIO_LED, "LED");
    gpio_direction_output(GPIO_LED, 0);

    btn_irq = gpio_to_irq(GPIO_BTN);
    irq = request_irq(btn_irq, &btn_interrupt, IRQF_TRIGGER_FALLING, "SWITCH", NULL);
    
    if(irq < 0)
        printk(KERN_ERR "%s: Request for IRQ %d failed\n", __FUNCTION__, GPIO_BTN);
    
    printk("init module, GPIO major number : %d\n", MOD_MAJOR);
    
    return 0;
}

void gpiobtn_exit(void){
    unregister_chrdev(MOD_MAJOR, MOD_NAME );
    free_irq(btn_irq, NULL);
    gpio_free(GPIO_BTN);
    
    printk("%s DRIVER CLEANUP\n", MOD_NAME);
    
    return;

}

module_init(gpiobtn_init);
module_exit(gpiobtn_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("melee");
MODULE_DESCRIPTION("Raspberry Pi 3 GPIO Switch Device Driver Module");
