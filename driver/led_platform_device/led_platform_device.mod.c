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
	{ 0xe651bb0c, "platform_driver_unregister" },
	{ 0x5922499a, "platform_device_unregister" },
	{ 0xd2e82297, "__platform_driver_register" },
	{ 0x93dca9c2, "platform_device_register" },
	{ 0x5a9d4c66, "misc_register" },
	{ 0x73c0cae3, "devm_ioremap" },
	{ 0xb1c1807, "platform_get_resource" },
	{ 0xecdb0401, "devm_iounmap" },
	{ 0x836ebef0, "misc_deregister" },
	{ 0x28118cb6, "__get_user_1" },
	{ 0x92997ed8, "_printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "1EC9F3279433E6EF3E32731");
