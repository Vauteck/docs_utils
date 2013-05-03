/* **************** LDD:1.0 s_03/trivial_ancient.c **************** */
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
#include <linux/module.h>

int __init init_module (void)
{
    printk (KERN_INFO "Hello: init_module loaded at 0x%p\n", init_module);
    return 0;
}
void __exit cleanup_module (void)
{
    printk (KERN_INFO "Bye: cleanup_module loaded at 0x%p\n", cleanup_module);
}
