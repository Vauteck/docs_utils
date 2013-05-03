/* **************** LDD:1.0 s_10/lab1_sem1.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein and the Linux Foundation, 2009
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@linuxfoundation.org
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
 * Semaphore Contention
 *
 * Write three simple modules where the second and third one use
 * a variable exported from the first one.  The second and third one
 * can be identical; just give them different names.

 * Hint: You can use the macro __stringify(KBUILD_MODNAME) to print
 * out the module name.

 * You can implement this by making small modifications to your
 * results from the modules exercise.

 * The exported variable should be a semaphore.  Have the first module
 * initialize it in the unlocked state.

 * The second (third) module should attempt to lock the semaphore, and
 * if it is locked, fail to load; make sure you return the appropriate
 * value from your initialization function.

 * Make sure you release the semaphore in your cleanup function.

 * Test by trying to load both modules simultaneously, and see if it is possible.
 * Make sure you can load one of the modules after the other has been unloaded, to
 * make sure you released the semaphore properly.

 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <asm/atomic.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif

DECLARE_MUTEX (my_sem);
EXPORT_SYMBOL (my_sem);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#define PRINT_COUNT(a) printk(KERN_INFO "semaphore_count=%d\n", atomic_read(&a));
#else
#define PRINT_COUNT(a) printk(KERN_INFO "semaphore_count=%u\n", a);
#endif

static int __init my_init (void)
{
    printk (KERN_INFO "\nInitializing semaphore, ");
    PRINT_COUNT (my_sem.count);
    return 0;
}

static void __exit my_exit (void)
{
    printk (KERN_INFO "\nExiting semaphore, ");
    PRINT_COUNT (my_sem.count);
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_10/lab1_sem1.c");
MODULE_LICENSE ("GPL v2");
