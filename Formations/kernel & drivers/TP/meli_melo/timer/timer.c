/************************************************************************\

  Ecriture de l'heure precise toutes les secondes.

\************************************************************************/

#include <linux/module.h>

static void fonction_periodique (unsigned long);

static struct timer_list timer;

static int __init chargement (void)
{
	init_timer (& timer);

	timer.function = fonction_periodique;
	timer.data = 0; /* inutilise */
	timer.expires = jiffies + HZ;
	add_timer(& timer);

	return 0;
}
 
static void __exit dechargement (void)
{
	del_timer(& timer);
}

static void fonction_periodique(unsigned long inutile)
{
	struct timeval time_of_day;

	do_gettimeofday(& time_of_day);
	printk(KERN_INFO "time_of_day: %ld.%06ld\n",
	                  time_of_day.tv_sec, time_of_day.tv_usec);

	mod_timer(& timer, jiffies+HZ);
}
module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");
