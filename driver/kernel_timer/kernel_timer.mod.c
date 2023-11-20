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
	{ 0x45b0d854, "cdev_del" },
	{ 0x4a1b0727, "device_destroy" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0xfe146ed6, "class_destroy" },
	{ 0x29ea4f1c, "device_create" },
	{ 0x33c2febe, "__class_create" },
	{ 0x1727426a, "cdev_add" },
	{ 0x8ab0a071, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0x526c3a6c, "jiffies" },
	{ 0x7f02188f, "__msecs_to_jiffies" },
	{ 0x92997ed8, "_printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "687A7638063D9DDE89B6C1F");
