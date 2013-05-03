/* **************** LDD:1.0 s_07/lab1_module2.c **************** */
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
 * Modules and Exporting Symbols: Module 2
 *
 * Write a pair of modules where the second one calls a
 * function in the first one.

 * Load them in the correct order and make sure all symbols are
 * resolved.
 @*/

#include <linux/module.h>
#include <linux/init.h>

extern void mod1fun (void);

static int __init my_init (void)
{
    printk (KERN_INFO "Hello world from mod2\n");
    mod1fun ();
    return 0;
}

static void __exit my_exit (void)
{
    printk (KERN_INFO "Goodbye world from mod2\n");
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_07/lab1_module2.c");
MODULE_LICENSE ("GPL v2");
