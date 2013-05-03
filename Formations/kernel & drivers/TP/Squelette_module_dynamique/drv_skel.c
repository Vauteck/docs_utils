#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int skel_init(void)
{
	printk(KERN_ALERT "Hello, world\n");
	return 0;
}

static void skel_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(skel_init);
module_exit(skel_exit);
