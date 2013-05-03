/* **************** CLDD:1.0 s_11/lab7_unlock.c **************** */
/*
 * The code herein is: Copyright 1998-2009 by Jerry Cooperstein
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 * URL:    http://www.coopj.com
 * email:  coop@coopj.com 
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */

/*
 * Mutex Locking from a Timer.
 *
 * Write a simple module that loads a timer and takes out a mutex and
 * then releases it when the timer runs.
 *
 * Doing this in an interrupt handler is supposed to be illegal. Here
 * we have a softirq context; is that illegal too?  Is this ignored,
 * enforced, or warned against?
 @*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/timer.h>
static DEFINE_MUTEX (my_mutex);

static struct timer_list timr;
static void ktfun (unsigned long var)
{
    printk (KERN_INFO "timer executing at jiffies=%ld\n", jiffies);
    mutex_lock (&my_mutex);
    printk (KERN_INFO "\nInit mutex in locked state, count=%d:\n",
            atomic_read (&my_mutex.count));
    mutex_unlock (&my_mutex);
    printk (KERN_INFO "\n mutex unlocked in timer, count=%d:\n",
            atomic_read (&my_mutex.count));
}

static int __init my_init (void)
{
    init_timer (&timr);
    timr.expires = jiffies + 3 * HZ;    /*3 seconds */
    timr.function = ktfun;
    add_timer (&timr);
    return 0;
}
static void __exit my_exit (void)
{
    /* delete any running timers */
    printk (KERN_INFO "Deleted timer A: rc = %d\n", del_timer_sync (&timr));
    printk (KERN_INFO "\nExiting with  mutex having count=%d:\n",
            atomic_read (&my_mutex.count));
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("CLDD:1.0 s_11/lab7_unlock.c");
MODULE_LICENSE ("GPL v2");
