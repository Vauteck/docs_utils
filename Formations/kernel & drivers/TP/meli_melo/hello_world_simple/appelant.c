/************************************************************************\

  Affichage au chargement et dechargement des PID et PPID de l'appelant

\************************************************************************/

#include <linux/module.h>
#include <linux/sched.h>

static int __init chargement (void)
{
	printk(KERN_INFO "Chargement par %u, son pere est %u\n",
	          current->pid, current->parent->pid);
	return 0;
}
 
static void __exit dechargement (void)
{
	printk(KERN_INFO "Dechargement par %u, son pere est %u\n",
	          current->pid, current->parent->pid);
}

module_init(chargement);
module_exit(dechargement);

MODULE_LICENSE("GPL");
