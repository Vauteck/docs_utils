#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x427e0e0b, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x6980fe91, "param_get_int" },
	{ 0x153c9ed4, "__register_chrdev" },
	{ 0xff964b25, "param_set_int" },
	{ 0xb72397d5, "printk" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0x37a0cba, "kfree" },
	{ 0x362ef408, "_copy_from_user" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "CFC4D9D08EECAB190FE9801");
