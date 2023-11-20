
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#include <linux/of_device.h>

static int __iomem *g_ioremap_addr;

#define GPFSEL1 (0x04/4)
#define GPSET0 (0x1C/4)
#define GPCLR0 (0x28/4)
#define GPIO_LED 18 // BCD_GPIO #18


static struct platform_driver my_platform_driver;

/* Turn on/off the led with led_app, use copy_from_user() */
static ssize_t my_dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char *led_on = "on"; 
	char *led_off = "off"; 
	unsigned char myled_value[10]; 
	unsigned int temp;

	pr_info("my_dev_write() is called.\n");

	pr_info("my_dev_write() is exit.\n");
	return 0;
}

static int my_dev_open(struct inode *inode, struct file *file)
{
	pr_info("my_dev_open() is called.\n");
	return 0;
}

static int my_dev_close(struct inode *inode, struct file *file)
{
	pr_info("my_dev_close() is called.\n");
	return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info("my_dev_ioctl() is called. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

static ssize_t my_dev_read(struct file* file, char* buf, size_t length, loff_t* ofs)
{
    int i = 0;

	printk("%s\n", __FUNCTION__);
	for(i=0;i<10;i++)
	{
	*(g_ioremap_addr + GPSET0) |= 1 << (GPIO_LED);
	mdelay(1000);
	*(g_ioremap_addr + GPCLR0) |= 1 << (GPIO_LED);
	mdelay(1000);
	}
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
    
// //============================================================
    // struct resource {
    //  const char *name;
    //  unsigned long start, end;
    //  unsigned long flags;
    //  struct resource *parent, *sibling, *child;
    // };
//       name : resource 이름을 지정.
//       start : rscource의 시작 값.
//       end : rscource의 마지막 값.
//       flags : resource type (IORESOURCE_IO, IORESOURCE_MEM, IORESOURCE_MEM, IORESOURCE_IRQ, 
//                              IORESOURCE_DMA)
      
    //-------------------------------------------------------------------
    pr_info("dev->num_resources = %d\n", pdev->num_resources); 
    struct resource *tmp_r = &pdev->resource[0];
    pr_info("tmp_r->start = 0x%08lx\n", (unsigned long)tmp_r->start);

    struct resource *tmp_r1 = &pdev->resource[1];
    pr_info("tmp_r1->start = 0x%08lx\n", (unsigned long)tmp_r1->start);    
    //-------------------------------------------------------------------
    
	/* get our first memory resource from device tree */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r) {
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}
	pr_info("r->start = 0x%08lx\n", (unsigned long)r->start);
	pr_info("r->end = 0x%08lx\n", (unsigned long)r->end);

    
    g_ioremap_addr = devm_ioremap(dev, r->start, resource_size(r));
	if (!g_ioremap_addr) {
		pr_err("ioremap failed \n");
		return -ENOMEM;
	}    
    else
    {
         pr_info("g_ioremap_addr = 0x%08lx\n", (unsigned long)g_ioremap_addr);  
    }
    
    *(g_ioremap_addr + GPFSEL1) &= ~(0x07 << 24);
    //output setting
    *(g_ioremap_addr + GPFSEL1) |= (0x01 << 24);
   

//============================================================


	ret_val = misc_register(&helloworld_miscdevice);
	if (ret_val != 0) {
		pr_err("could not register the misc device mydev");
		return ret_val;
	}
	pr_info("mydev: got minor %i\n",helloworld_miscdevice.minor);
	return 0;
}

static int __exit my_remove(struct platform_device *pdev)
{
	misc_deregister(&helloworld_miscdevice);
	pr_info("platform_remove exit\n");
	return 0;
}

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "haha_device",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is a platform driver that turns on/off \
	the LED using sysfs and an user application");


