#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("Dual BSD/GPL");

static char* whom    = "world";
static int   howmany = 1;

module_param(howmany, int,   S_IRUGO);
module_param(whom,    charp, S_IRUGO);

static int param_init(void)
{
	int i;
	for (i = 0; i < howmany; i++)
	{
		printk(KERN_ALERT "(%d) Hello, %s\n", i, whom);
	}
	
	return 0;
}

static void param_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(param_init);
module_exit(param_exit);
