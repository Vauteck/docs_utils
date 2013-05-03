/************************************************************************\

  Ecriture au chargement et dechargement du nombre de jiffies ecoules
  depuis le boot.

\************************************************************************/

#include <linux/module.h>
#include <linux/jiffies.h>

static int __init chargement (void)
{
	printk(KERN_INFO "HZ=%d, jiffies=%ld\n", HZ, jiffies);
	return 0;
}
 
static void __exit dechargement (void)
{
	printk(KERN_INFO "HZ=%d, jiffies=%ld\n", HZ, jiffies);
}

module_init(chargement);
module_exit(dechargement);

MODULE_LICENSE("GPL");

