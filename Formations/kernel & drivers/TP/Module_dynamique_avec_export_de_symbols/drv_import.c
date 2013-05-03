#include <linux/module.h>

#include "export_api.h"

static int __init import_chargement(void)
{
	printk(KERN_ALERT "Hello, world\n");  
	
	fonction_hello(10);
	
	return 0;
}

static void __exit import_dechargement(void)
{
	fonction_hello(20);
	
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(import_chargement);
module_exit(import_dechargement);

MODULE_LICENSE("GPL");
