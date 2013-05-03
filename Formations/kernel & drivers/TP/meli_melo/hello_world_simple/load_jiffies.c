/************************************************************************\

  Ecriture au chargement et dechargement des heures precises.

\************************************************************************/

#include <linux/module.h>
#include <linux/jiffies.h>

static int __init chargement (void)
{
	struct timeval  tv_jif, tv_tod;
	struct timespec ts_ckt, ts_tod;

	jiffies_to_timeval(jiffies, & tv_jif);
	do_gettimeofday(& tv_tod);
	ts_ckt = current_kernel_time();
	getnstimeofday(& ts_tod);
	printk(KERN_INFO  "Chargement :\n");
	printk(KERN_INFO "tv_jif.tv_sec = %ld, tv_jif.tv_usec = %ld\n",
	                  tv_jif.tv_sec, tv_jif.tv_usec);
	printk(KERN_INFO "tv_tod.tv_sec = %ld, tv_tod.tv_usec = %ld\n",
	                  tv_tod.tv_sec, tv_tod.tv_usec);
	printk(KERN_INFO "ts_ckt.tv_sec = %ld, ts_ckt.tv_nsec = %ld\n",
	                  ts_ckt.tv_sec, ts_ckt.tv_nsec);
	printk(KERN_INFO "ts_tod.tv_sec = %ld, ts_tod.tv_nsec = %ld\n",
	                  ts_tod.tv_sec, ts_tod.tv_nsec);
	return 0;
}
 
static void __exit dechargement (void)
{
	struct timeval tv_jif, tv_tod;
	struct timespec ts_ckt, ts_tod;

	jiffies_to_timeval(jiffies, & tv_jif);
	do_gettimeofday(& tv_tod);
	ts_ckt = current_kernel_time();
	getnstimeofday(& ts_tod);

	printk(KERN_INFO  "Dechargement :\n");
	printk(KERN_INFO "tv_jif.tv_sec = %ld, tv_jif.tv_usec = %ld\n",
	                  tv_jif.tv_sec, tv_jif.tv_usec);
	printk(KERN_INFO "tv_tod.tv_sec = %ld, tv_tod.tv_usec = %ld\n",
	                  tv_tod.tv_sec, tv_tod.tv_usec);
	printk(KERN_INFO "ts_ckt.tv_sec = %ld, ts_ckt.tv_nsec = %ld\n",
	                  ts_ckt.tv_sec, ts_ckt.tv_nsec);
	printk(KERN_INFO "ts_tod.tv_sec = %ld, ts_tod.tv_nsec = %ld\n",
	                  ts_tod.tv_sec, ts_tod.tv_nsec);
}

module_init(chargement);
module_exit(dechargement);

MODULE_LICENSE("GPL");

