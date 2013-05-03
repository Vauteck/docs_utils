/* **************** CLDD:1.0 s_12/lab1_sem3.c **************** */
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
 * second and third module to test semaphore contention.
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <asm/atomic.h>
#include <linux/errno.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif

static char *modname = __stringify (KBUILD_BASENAME);

extern struct semaphore my_sem;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#define PRINT_COUNT(a) printk(KERN_INFO "sempaphore_count=%d\n", atomic_read(&a));
#else
#define PRINT_COUNT(a) printk(KERN_INFO "sempaphore_count=%u\n", a);
#endif

static int __init my_init (void)
{
    printk (KERN_INFO "Trying to load module %s\n", modname);
    PRINT_COUNT (my_sem.count);

    if (down_trylock (&my_sem)) {
        printk (KERN_WARNING "Not loading the module; down_trylock() failed\n");
        return -EBUSY;
    }
    printk (KERN_INFO "\nGrabbed semaphore in %s, ", modname);
    PRINT_COUNT (my_sem.count);

    return 0;
}

static void __exit my_exit (void)
{
    up (&my_sem);
    printk (KERN_INFO "\nExiting semaphore in %s, ", modname);
    PRINT_COUNT (my_sem.count);
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("CLDD:1.0 s_12/lab1_sem3.c");
MODULE_LICENSE ("GPL v2");
