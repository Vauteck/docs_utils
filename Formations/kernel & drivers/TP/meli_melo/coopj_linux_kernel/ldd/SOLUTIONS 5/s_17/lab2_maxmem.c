/* **************** CLDD:1.0 s_17/lab2_maxmem.c **************** */
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
 * Testing Maximum Memory Allocation
 *
 * See how much memory you can obtain dynamically, using both
 * kmalloc() and __get_free_pages().
 *
 * Start with requesting 1 page of memory, and then keep doubling
 * until *your request fails for each type fails.
 *
 * Make sure you free any memory you receive.
 *
 * You'll probably want to use GFP_ATOMIC rather than
 * GFP_KERNEL. (Why?)

 * If you have trouble getting enough memory due to memory
 * fragmentation trying writing a poor-man's defragmenter, and then
 * running again.  The defragmenter can just be an application that
 * grabs all available memory, uses it, and then releases it when
 * done, thereby clearing the caches.  You can also try the command
 * sync; echo 3 > /proc/sys/vm/drop_caches .

 * Try the same thing with vmalloc().  Rather than
 * doubling allocations, start at 4 MB and increase in 4 MB incremements
 * until failure results.  Note this may hang while loading. (Why?)
 @*/

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

static int __init my_init (void)
{
    static char *kbuff;
    static unsigned long order;
    int size;

    /* try __get_free_pages__ */

    for (size = PAGE_SIZE, order = 0; order < MAX_ORDER; order++, size *= 2) {
        printk (KERN_INFO " order=%2ld, pages=%5ld, size=%8d ", order,
                size / PAGE_SIZE, size);
        if (!(kbuff = (char *)__get_free_pages (GFP_ATOMIC, order))) {
            printk (KERN_ERR "... __get_free_pages failed\n");
            break;
        }
        printk (KERN_INFO "... __get_free_pages OK\n");
        free_pages ((unsigned long)kbuff, order);
    }

    /* try kmalloc */

    for (size = PAGE_SIZE, order = 0; order < MAX_ORDER; order++, size *= 2) {
        printk (KERN_INFO " order=%2ld, pages=%5ld, size=%8d ", order,
                size / PAGE_SIZE, size);
        if (!(kbuff = (char *)kmalloc ((size_t) size, GFP_ATOMIC))) {
            printk (KERN_ERR "... kmalloc failed\n");
            break;
        }
        printk (KERN_INFO "... kmalloc OK\n");
        kfree (kbuff);
    }
    return 0;
}
static void __exit my_exit (void)
{
    printk (KERN_INFO "Module Unloading\n");
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("CLDD:1.0 s_17/lab2_maxmem.c");
MODULE_LICENSE ("GPL v2");
