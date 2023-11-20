#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/io.h> // ioremap(), iounmap()

#define GPIO_BASE 0xFE200000
volatile unsigned static int* gpio_addr;
#define GPFSEL1 (0x04/4)
#define GPSET0 (0x1C/4)
#define GPCLR0 (0x28/4)
#define GPIO_LED 18 // BCD_GPIO #18

// 사용자 구현 함수 : 열기, 닫기, 읽기, 쓰기, ioctl
static int drv_hello_open(struct inode* inode, struct file* file)
{
    printk("%s\n", __FUNCTION__);

    return 0;
}

static int drv_hello_release(struct inode* inode, struct file* file)
{
    printk("%s\n", __FUNCTION__);

    return 0;
}

static ssize_t drv_hello_read(struct file* file, char* buf, size_t length, loff_t* ofs)
{
    printk("%s\n", __FUNCTION__);

    return 0;
}

static ssize_t drv_hello_write(struct file* file, const char* buf, size_t length, loff_t* ofs)
{
    printk("%s\n", __FUNCTION__);
    unsigned char c;

    get_user(c, buf);
   
    
    *(gpio_addr + GPFSEL1) &= ~(0x07 << 24);
    //output setting
    *(gpio_addr + GPFSEL1) |= (0x01 << 24);

    if (c == 1)
    {
        //	*(g_ioremap_addr + GPSET0) |= 1 << (GPIO_LED);
        *(gpio_addr + GPSET0) |= 1 << (GPIO_LED);
        pr_info("my_dev_write() is called set %c.\n", c);
    }
    else
    {
        //	*(g_ioremap_addr + GPCLR0) |= 1 << (GPIO_LED);
        *(gpio_addr + GPCLR0) |= 1 << (GPIO_LED);
        pr_info("my_dev_write() is called clear %c.\n", c);
        //*(gpio_addr + GPSET0) &= ~(0x01 << GPIO_LED);
    }
    //1 for on, 0 for off.
    return 0;
}

//static DEFINE_MUTEX(drv_hello_mutex);
static long drv_hello_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
    printk("%s\n", __FUNCTION__);

   // mutex_lock(&drv_hello_mutex);
    switch (cmd) {
    default:
     //   mutex_unlock(&drv_hello_mutex);
        return ENOTTY;
    }

    //mutex_unlock(&drv_hello_mutex);
    return 0;
}

static struct file_operations drv_hello_fops =
{
    .owner = THIS_MODULE,

    // 사용자 구현 함수 연결
    .open = drv_hello_open,
    .release = drv_hello_release,
    .read = drv_hello_read,
    .write = drv_hello_write,
    .unlocked_ioctl = drv_hello_ioctl,
};

static struct miscdevice drv_hello_driver =
{
    .minor = MISC_DYNAMIC_MINOR,
    // 드라이버 파일 이름 설정
    .name = "drv_hello",
    // file_operations 구조체 등록
    .fops = &drv_hello_fops,
};

static int drv_hello_init(void)
{
    printk("%s\n", __FUNCTION__);

    gpio_addr = ioremap(GPIO_BASE, BLOCK_SIZE);

    *(gpio_addr + GPFSEL1) &= ~(0x07 << 24);
    //output setting
    *(gpio_addr + GPFSEL1) |= (0x01 << 24);

    // 디바이스 드라이버가 적재될때 등록
    // miscdevice 구조체 연결
    return misc_register(&drv_hello_driver);
}

static void drv_hello_exit(void)
{
    printk("%s\n", __FUNCTION__);

    // 모듈이 제거될때 노드 파일 제거
    misc_deregister(&drv_hello_driver);

}

module_init(drv_hello_init);
module_exit(drv_hello_exit);

MODULE_AUTHOR("PlanX Studio");
MODULE_DESCRIPTION("drv_hello");
MODULE_LICENSE("Dual BSD/GPL");






