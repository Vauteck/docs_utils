/************************************************************************\
  module_horodateur_sol

  Module envoyant dans les traces du systeme l'heure precise de ses
  chargement et dechargement.

\************************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

static int __init chargement (void)
{
	struct timespec heure = current_kernel_time();
	
	printk(KERN_INFO "Chargement a %ld sec et %ld nsec\n",
		heure.tv_sec, heure.tv_nsec);

	return 0; 
}
 
 
static void __exit liberation (void)
{
	struct timespec heure = current_kernel_time();
	
	printk(KERN_INFO "Extraction a %ld sec et %ld nsec\n",
		heure.tv_sec, heure.tv_nsec);
}

module_init(chargement);
module_exit(liberation);

MODULE_LICENSE("GPL");
