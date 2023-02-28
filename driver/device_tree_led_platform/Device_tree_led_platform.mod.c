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
	{ 0xd2e82297, "__platform_driver_register" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x5a9d4c66, "misc_register" },
	{ 0x649a9fe4, "device_property_read_string" },
	{ 0x1d37eeed, "ioremap" },
	{ 0x77304b46, "_dev_info" },
	{ 0x5b434143, "_dev_err" },
	{ 0xc6491dd3, "device_property_read_u32_array" },
	{ 0xca22669, "device_property_present" },
	{ 0xc34c13d8, "of_match_device" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0x92997ed8, "_printk" },
	{ 0x836ebef0, "misc_deregister" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cbrightlight,haha_device");
MODULE_ALIAS("of:N*T*Cbrightlight,haha_deviceC*");

MODULE_INFO(srcversion, "7C1E7B25216F00B3A1A8A8B");
