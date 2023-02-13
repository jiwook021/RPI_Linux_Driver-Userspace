#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xabab44ce, "module_layout" },
	{ 0x4a1b0727, "device_destroy" },
	{ 0x37a0cba, "kfree" },
	{ 0xea3c74e, "tasklet_kill" },
	{ 0x45b0d854, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xfe146ed6, "class_destroy" },
	{ 0xf76cd0a5, "sysfs_remove_file_ns" },
	{ 0xa54133aa, "kobject_put" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x2364c85a, "tasklet_init" },
	{ 0x39624ead, "kmem_cache_alloc_trace" },
	{ 0xd0295e98, "kmalloc_caches" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x90e7185b, "gpiod_to_irq" },
	{ 0x73d7dd02, "gpiod_direction_output_raw" },
	{ 0xb5df8513, "gpiod_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x1ecdc082, "sysfs_create_file_ns" },
	{ 0x1ddc1254, "kobject_create_and_add" },
	{ 0x88d821ec, "kernel_kobj" },
	{ 0x29ea4f1c, "device_create" },
	{ 0x33c2febe, "__class_create" },
	{ 0x1727426a, "cdev_add" },
	{ 0x8ab0a071, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xf9a482f9, "msleep" },
	{ 0xbd5adb92, "gpiod_set_raw_value" },
	{ 0xa3202c1, "gpio_to_desc" },
	{ 0x9d2ab8ac, "__tasklet_schedule" },
	{ 0xca54fee, "_test_and_set_bit" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0x92997ed8, "_printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "0BCA66DD02A5C69204FD09D");
