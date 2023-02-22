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
	{ 0x73d7dd02, "gpiod_direction_output_raw" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x5a9d4c66, "misc_register" },
	{ 0xbd5adb92, "gpiod_set_raw_value" },
	{ 0xa3202c1, "gpio_to_desc" },
	{ 0x28118cb6, "__get_user_1" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xfe990052, "gpio_free" },
	{ 0x836ebef0, "misc_deregister" },
	{ 0x92997ed8, "_printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cbrightlight,hahadev");
MODULE_ALIAS("of:N*T*Cbrightlight,hahadevC*");

MODULE_INFO(srcversion, "507A801FA4C0FE7BB25E365");
