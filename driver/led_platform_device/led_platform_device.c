#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>

/* Include header files for ioremap() and copy_to_user() */
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

#include <linux/of_device.h>

#define GPIO_BASE 0xFE200000
#define BLOCK_SIZE 4096
volatile unsigned int __iomem* g_ioremap_addr;

//volatile unsigned int* gpio_addr;
#define GPFSEL1 (0x04/4)
#define GPSET0 (0x1C/4)
#define GPCLR0 (0x28/4)
#define GPIO_LED 18 // BCD_GPIO #18

static ssize_t my_dev_write(struct file* file, const char __user* buff, size_t count, loff_t* ppos)
{
	unsigned char c;

	pr_info("my_dev_write() is called.\n");	
	get_user(c, buff);

	if (c == 1)
	{
		*(g_ioremap_addr + GPSET0) |= 1 << (GPIO_LED);
		//*(gpio_addr + GPSET0) |= 1 << (GPIO_LED);
		pr_info("my_dev_write() is called set %c.\n", c);
	}
	else
	{
		*(g_ioremap_addr + GPCLR0) |= 1 << (GPIO_LED);
		//*(gpio_addr + GPCLR0) |= 1 << (GPIO_LED);
		pr_info("my_dev_write() is called clear %c.\n", c);
		//*(gpio_addr + GPSET0) &= ~(0x01 << GPIO_LED);
	}

		//1 for on, 0 for off.
	return 0;
}

static ssize_t my_dev_read(struct file* file, char __user* buff, size_t count, loff_t* ppos)
{
	char buf[32];
	size_t size;

	pr_info("my_dev_read() is called.\n");

	return 0;
}

static int my_dev_open(struct inode* inode, struct file* file)
{
	pr_info("my_dev_open() is called.\n");
	return 0;
}

static int my_dev_close(struct inode* inode, struct file* file)
{
	pr_info("my_dev_close() is called.\n");
	return 0;
}

static long my_dev_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	pr_info("my_dev_ioctl() is called. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.write = my_dev_write,
	.read = my_dev_read,
	.release = my_dev_close,
	.unlocked_ioctl = my_dev_ioctl,
};

static struct miscdevice helloworld_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mydev",
	.fops = &my_dev_fops,
};


static int my_probe(struct platform_device* pdev)
{
	int retval;
	int i = 0;
	unsigned int temp;

	struct resource* r0;
	struct resource* r1;
	struct device* dev = &pdev->dev;


	pr_info("platform_probe enter\n");

	//=================================================================
	r0 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r0) {
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}
	pr_info("r0->start = 0x%08lx\n", (unsigned long)r0->start);
	pr_info("r0->end = 0x%08lx\n", (unsigned long)r0->end);

	  //   r1 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	 	//if (!r1) {
	 	//	pr_err("IORESOURCE_MEM, 1 does not exist\n");
	 	//	return -EINVAL;
	 	//}
	 	//pr_info("r1->start = 0x%08lx\n", (unsigned long)r1->start);
	 	//pr_info("r1->end = 0x%08lx\n", (unsigned long)r1->end);
	//=================================================================    

	//gpio_addr = ioremap(GPIO_BASE, BLOCK_SIZE);
	g_ioremap_addr = devm_ioremap(dev, r0->start, resource_size(r0));
	if (!g_ioremap_addr) {
		pr_err("ioremap failed \n");
		return -ENOMEM;
	}
	else
	{
		pr_info("g_ioremap_addr = 0x%08lx\n", (unsigned long)g_ioremap_addr);
	}

	//GPIO clear
	*(g_ioremap_addr + GPFSEL1) &= ~(0x07 << 24);
	//output setting
	*(g_ioremap_addr + GPFSEL1) |= (0x01 << 24);

	//*(gpio_addr + GPFSEL1) &= ~(0x07 << 24);
	////output setting
	//*(gpio_addr + GPFSEL1) |= (0x01 << 24);


	retval = misc_register(&helloworld_miscdevice);
	if (retval)
		return retval; /* misc_register returns 0 if success */
	pr_info("mydev: got minor %i\n", helloworld_miscdevice.minor);
	return 0;
}

static int __exit my_remove(struct platform_device* pdev)
{
	misc_deregister(&helloworld_miscdevice);
	//iounmap(gpio_addr);
	devm_iounmap(&pdev->dev,g_ioremap_addr);
	pr_info("platform_remove exit\n");
	return 0;
}


static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "hahadev",
		.owner = THIS_MODULE,
	}
};



// static struct resource haha_plat_dev_rsrc[] = {
// 	{
// 		/* addr */
// 		.start	= 0x7E200000,
// 		.end	= 0x7E200100,
// 		.flags	= IORESOURCE_MEM,
// 	}, {
// 		/* data */
// 		.start	= 0x7E200100,
// 		.end	= 0x7E200200,
// 		.flags	= IORESOURCE_MEM,
// 	}, {
// 		.flags	= IORESOURCE_IRQ
// 			| IORESOURCE_IRQ_HIGHEDGE /* rising (active high) */,
// 	},
// };

static struct resource haha_plat_dev_rsrc[] = {
	{
		/* addr */
		.start = 0xFE200000,
		.end = 0xFE200100,
		.flags = IORESOURCE_MEM,
	},{
		.flags = IORESOURCE_IRQ
			| IORESOURCE_IRQ_HIGHEDGE /* rising (active high) */,
	},
};

static struct platform_device* haha_device;

static struct platform_device haha_plat_dev = {
	.name = "hahadev",
	.resource = haha_plat_dev_rsrc,
	.num_resources = ARRAY_SIZE(haha_plat_dev_rsrc),
};

static struct platform_device* devices[] __initdata = {
 &haha_plat_dev ,
};


static int __init demo_init(void)
{
	int ret_val;
	int res = 0;

	pr_info("demo_init enter\n");

	platform_device_register(&haha_plat_dev);

	ret_val = platform_driver_register(&my_platform_driver);
	if (ret_val != 0) {
		pr_err("platform value returned %d\n", ret_val);
		return ret_val;
	}
	pr_info("demo_init exit\n");
	return 0;
}

static void __exit demo_exit(void)
{
	pr_info("demo_exit enter\n");

	platform_device_unregister(&haha_plat_dev);
	platform_driver_unregister(&my_platform_driver);
	

	pr_info("demo_exit exit\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_AUTHOR("Chunghan Yi <chunghan.yi@gmail.com>");
MODULE_DESCRIPTION("This is a platform driver that turns on/off a led \
	when probing and read MAC address");
