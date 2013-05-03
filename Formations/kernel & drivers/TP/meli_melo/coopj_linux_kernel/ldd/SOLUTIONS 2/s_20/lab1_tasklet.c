/* **************** LDD:1.0 s_20/lab1_tasklet.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2009
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
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
 * Deferred Functions (tasklet solution)
 *
 * Write a driver that schedules a deferred function whenever a
 * write() to the device takes place.
 *
 * Pass some data to the driver and have it print out.
 *
 * Have it print out the current->pid field when the tasklet is
 * scheduled, and then again when the queued function is executed.
 *
 * Implement this using:
 *
 * tasklets
 * work queues
 *
 * You can use the same testing programs you used in the sleep
 * exercises.
 *
 * Try scheduling multiple deferred functions
 * and see if they come out in LIFO or FIFO order.  What happens if you
 * try to schedule the deferred function more than once?
 *
 @*/

#include <linux/module.h>
#include <linux/interrupt.h>
#include "lab_char.h"

static struct simp
{
    int len;
} t_data = {
.len = 100};

static void t_fun (unsigned long t_arg)
{
    struct simp *datum;
    datum = (struct simp *)t_arg;
    printk (KERN_INFO "I am in t_fun, jiffies = %ld\n", jiffies);
    printk (KERN_INFO " I think my current task pid is %d\n",
            (int)current->pid);
    printk (KERN_INFO " my data is: %d\n", datum->len);
}

/* initialize tasklet */

static DECLARE_TASKLET (t_name, t_fun, (unsigned long)&t_data);

static ssize_t
mycdrv_write (struct file *file, const char __user * buf, size_t lbuf,
              loff_t * ppos)
{
    printk (KERN_INFO " Entering the WRITE function\n");
    printk (KERN_INFO " my current task pid is %d\n", (int)current->pid);
    printk (KERN_INFO "about to schedule tasklet, jiffies=%ld\n", jiffies);
    tasklet_schedule (&t_name);

    printk (KERN_INFO " i queued the task, jiffies=%ld\n", jiffies);
    t_data.len += 100;
    return lbuf;
}

static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mycdrv_generic_read,
    .write = mycdrv_write,
    .open = mycdrv_generic_open,
    .release = mycdrv_generic_release,
};

module_init (my_generic_init);
module_exit (my_generic_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_20/lab1_tasklet.c");
MODULE_LICENSE ("GPL v2");
