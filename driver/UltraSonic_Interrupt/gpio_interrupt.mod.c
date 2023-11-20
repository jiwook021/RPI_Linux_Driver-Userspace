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
	{ 0x24d273d1, "add_timer" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x90e7185b, "gpiod_to_irq" },
	{ 0xb5df8513, "gpiod_direction_input" },
	{ 0x73d7dd02, "gpiod_direction_output_raw" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x29ea4f1c, "device_create" },
	{ 0x33c2febe, "__class_create" },
	{ 0x1727426a, "cdev_add" },
	{ 0x8ab0a071, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x1d37eeed, "ioremap" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x45b0d854, "cdev_del" },
	{ 0xfe146ed6, "class_destroy" },
	{ 0x4a1b0727, "device_destroy" },
	{ 0xfe990052, "gpio_free" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0x2cfde9a2, "warn_slowpath_fmt" },
	{ 0x5f754e5a, "memset" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x97255bdf, "strlen" },
	{ 0xb43f9365, "ktime_get" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0xa68613dd, "get_jiffies_64" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0xbd5adb92, "gpiod_set_raw_value" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0x49b76df7, "gpiod_get_raw_value" },
	{ 0xa3202c1, "gpio_to_desc" },
	{ 0x92997ed8, "_printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "ADD1FA46E1580640FD2D1E4");
