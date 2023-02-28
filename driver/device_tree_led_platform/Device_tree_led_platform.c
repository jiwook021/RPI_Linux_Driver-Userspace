#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#include <linux/of_device.h>

#define MAX_KEY 3

#define GPFSEL1 (0x04/4)
#define GPSET0 (0x1C/4)
#define GPCLR0 (0x28/4)
#define GPIO_LED 18 // BCD_GPIO #18

volatile unsigned int * gpio_addr;
static struct platform_driver my_platform_driver;

/* Turn on/off the led with led_app, use copy_from_user() */
static ssize_t device_tree_led_misc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char *led_on = "on"; 
	char *led_off = "off"; 
	unsigned char myled_value[10]; 
	unsigned int temp;

	pr_info("device_tree_led_misc_write() is called.\n");

	pr_info("device_tree_led_misc_write() is exit.\n");
	return 0;
}


static ssize_t device_tree_led_misc_read(struct file* file, char* buf, size_t length, loff_t* ofs)
{
    int i = 0;

	printk("%s\n", __FUNCTION__);
	for(i=0;i<10;i++)
	{
	*(gpio_addr + GPSET0) |= 1 << (GPIO_LED);
	mdelay(1000);
	*(gpio_addr + GPCLR0) |= 1 << (GPIO_LED);
	mdelay(1000);
	}
	return 0;
}

static int device_tree_led_misc_open(struct inode *inode, struct file *file)
{
	pr_info("device_tree_led_misc_open() is called.\n");
	return 0;
}

static int device_tree_led_misc_close(struct inode *inode, struct file *file)
{
	pr_info("device_tree_led_misc_close() is called.\n");
	return 0;
}

static long device_tree_led_misc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info("device_tree_led_misc_ioctl() is called. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

static const struct file_operations device_tree_led_misc_fops = {
	.owner = THIS_MODULE,
	.open = device_tree_led_misc_open,
	.write = device_tree_led_misc_write,
	.read = device_tree_led_misc_read,
	.release = device_tree_led_misc_close,
	.unlocked_ioctl = device_tree_led_misc_ioctl,
};

static struct miscdevice device_tree_led_misc_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mydev",
	.fops = &device_tree_led_misc_fops,
};


static const struct of_device_id my_of_ids[] = {
	{ .compatible = "brightlight,haha_device"},      
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);


static int my_probe(struct platform_device *pdev)
{
	int ret_val;
	struct resource *r;
	struct device *dev = &pdev->dev;
	int i;
	unsigned int temp;

	pr_info("platform_probe enter\n");

    const struct of_device_id *of_id = of_match_device(my_of_ids, &pdev->dev);
    if (!of_id) {
        pr_err("of match error\n");
        return -EINVAL;
    }
    
//---------------------------------------------------------------------    
	/* Check for device properties */
    const char *label;
	int haha_value, ret;
    u64 reg_addr, reg_length;
    int max_key, reg_max_key;
    u32 reg_value[2];

    
	if(!device_property_present(dev, "reg")) {
		printk("dt_probe - Error! Device property 'label' not found!\n");
		return -1;
	}
    
    
	/* parse key map */
	r = device_property_read_u32_array(dev, "reg", NULL, MAX_KEY);
    
	if (r > 0 && r <= MAX_KEY) {
		reg_max_key = r;
		r = device_property_read_u32_array(dev, "reg", &reg_value[0], reg_max_key);
		if (r)
			dev_err(dev, "Failed get key map info\n");
	} else {
		dev_info(dev, "No key map found\n");
	}    
    pr_info("reg_addr = 0x%08lx\n", (unsigned long)reg_value[0]);   
    pr_info("reg_length = 0x%08lx\n", (unsigned long)reg_value[1]);    
    
    gpio_addr = ioremap(reg_value[0], reg_value[1]);
    pr_info("gpio_addr = 0x%08lx\n", (unsigned long)gpio_addr); 
    *(gpio_addr + GPFSEL1) &= ~(0x07 << 24);
    //output setting
    *(gpio_addr + GPFSEL1) |= (0x01 << 24);
    
	/* Read device properties */
	ret = device_property_read_string(dev, "label", &label);
	if(ret) {
		printk("dt_probe - Error! Could not read 'label'\n");
		return -1;
	}
	printk("dt_probe - label: %s\n", label);
    
	/* Read device properties */
	ret = device_property_read_u32(dev, "haha_value", &haha_value);
	if(ret) {
		printk("dt_probe - Error! Could not read 'haha_value'\n");
		return -1;
	}
	printk("dt_probe - haha_value: %d\n", haha_value);
//---------------------------------------------------------------------    

	ret_val = misc_register(&device_tree_led_misc_miscdevice);
	if (ret_val != 0) {
		pr_err("could not register the misc device mydev");
		return ret_val;
	}

	

	pr_info("mydev: got minor %i\n",device_tree_led_misc_miscdevice.minor);
	return 0;
}

static int __exit my_remove(struct platform_device *pdev)
{
	misc_deregister(&device_tree_led_misc_miscdevice);
	pr_info("platform_remove exit\n");
	return 0;
}

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "hahadev",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is a platform driver that turns on/off \
	the LED using sysfs and an user application");

