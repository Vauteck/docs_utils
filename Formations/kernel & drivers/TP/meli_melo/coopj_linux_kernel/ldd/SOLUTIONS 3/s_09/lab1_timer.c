/* **************** LDD:1.0 s_09/lab1_timer.c **************** */
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
 * Kernel Timers from a Character Driver
 *
 * Write a driver that puts launches a kernel timer whenever a write
 * to the device takes place.
 *
 * Pass some data to the driver and have it print out.
 *
 * Have it print out the current->pid field when the timer function
 * is scheduled, and then again when the function is executed.
 *
 * You can use the same testing programs you used in the previous
 * exercise.
 @*/

#include <linux/module.h>
#include "lab_char.h"
#include <linux/timer.h>

static struct timer_list my_timer;

static void my_timer_function (unsigned long ptr)
{
    printk (KERN_INFO "I am in my_timer_fun, jiffies = %ld\n", jiffies);
    printk (KERN_INFO " I think my current task pid is %d\n",
            (int)current->pid);
    printk (KERN_INFO " my data is: %d\n", (int)ptr);
}

static ssize_t
mycdrv_write (struct file *file, const char __user * buf, size_t lbuf,
              loff_t * ppos)
{
    static int len = 100;
    printk (KERN_INFO " Entering the WRITE function\n");
    printk (KERN_INFO " my current task pid is %d\n", (int)current->pid);
    init_timer (&my_timer);     /* intialize */
    my_timer.function = my_timer_function;
    my_timer.expires = jiffies + HZ;    /* one second delay */
    /*  my_timer.data = (void *) len; */
    my_timer.data = len;
    printk (KERN_INFO "Adding timer at jiffies = %ld\n", jiffies);
    add_timer (&my_timer);
    len += 100;
    return lbuf;
}

static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mycdrv_generic_read,
    .write = mycdrv_write,
    .open = mycdrv_generic_open,
    .release = mycdrv_generic_release,
};

static void __exit my_exit (void)
{
    /* delete any running timers */
    printk (KERN_INFO "Deleted time,r rc = %d\n", del_timer_sync (&my_timer));
    my_generic_exit ();
}

module_init (my_generic_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_09/lab1_timer.c");
MODULE_LICENSE ("GPL v2");
