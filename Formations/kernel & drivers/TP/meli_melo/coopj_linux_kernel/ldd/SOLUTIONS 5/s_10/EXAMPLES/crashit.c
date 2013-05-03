/* **************** CLDD:1.0 s_10/crashit.c **************** */
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

#include <linux/module.h>
#include <linux/init.h>

static int __init my_init (void)
{
    int *i;
    i = 0;
    printk (KERN_INFO "Hello: init_module loaded at address 0x%p\n",
            init_module);
    printk (KERN_INFO "i=%d\n", *i);
    return 0;
}

static void __exit my_exit (void)
{
    printk (KERN_INFO "Hello: cleanup_module loaded at address 0x%p\n",
            cleanup_module);
}

module_init (my_init);
module_exit (my_exit);

MODULE_LICENSE ("GPL v2");
