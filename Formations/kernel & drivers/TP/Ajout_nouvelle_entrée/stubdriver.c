/*
 * TP03 : Ajout d'une nouvelle entrée dans le menu gnu/linux
 *
 * EUROGICIEL Rennes - Formation noyau & drivers sous gnu/linux
 */

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int stub_init(void)
{
	printk(KERN_ALERT "Hello, world\n");
	return 0;
}

static void stub_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(stub_init);
module_exit(stub_exit);
