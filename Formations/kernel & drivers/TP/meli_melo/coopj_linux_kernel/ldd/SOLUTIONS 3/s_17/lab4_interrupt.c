/* **************** LDD:1.0 s_17/lab4_interrupt.c **************** */
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
 * User-Space Interrupt Handling (driver)
 *
 * Adapt the character driver with polling to handle a shared
 * interrupt.
 *
 * The read method should sleep until events are available and then
 * deal with potentially multiple events.
 *
 * The information passed back by the read should include the number
 * of events.
 *
 * You can reuse the previously written testing program that opens the
 * device node and then sits on it with poll() until interrupts
 * arrive.
 *
 * You can also test it with just using the simple read program, or
 * doing cat < /dev/mycdrv and generating some interrupts.
 *
 * You can probably also implement a solution that does not involve
 * poll(), but just a blocking read.
 @*/

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include "lab_char.h"

static DECLARE_WAIT_QUEUE_HEAD (wq);

#define SHARED_IRQ 12
static int irq = SHARED_IRQ;
module_param (irq, int, S_IRUGO);

static atomic_t nevents;

static irqreturn_t my_interrupt (int irq, void *dev_id)
{
    atomic_inc (&nevents);
    wake_up_interruptible (&wq);
    return IRQ_NONE;
}

static ssize_t
mycdrv_read (struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
    char ret_str[32];
    int nev, nc;
    printk (KERN_INFO "waiting for interrupt at jiffies=%ld\n", jiffies);
    wait_event_interruptible (wq, (atomic_read (&nevents) > 0));
    if (signal_pending (current)) {
        printk (KERN_INFO "process %i woken up by a signal\n", current->pid);
        return -ERESTARTSYS;
    }
    nev = atomic_read (&nevents);
    printk (KERN_INFO "Now dealing with %d events\n", nev);
    atomic_sub (nev, &nevents);
    nc = sprintf (ret_str, "%d", nev);
    if (copy_to_user (buf, ret_str, nc))
        return -EIO;
    return nc;
}

static unsigned int mycdrv_poll (struct file *file, poll_table * wait)
{
    poll_wait (file, &wq, wait);
    printk (KERN_INFO "In poll at jiffies=%ld, nevents=%d\n", jiffies,
            atomic_read (&nevents));
    if (atomic_read (&nevents) > 0)
        return POLLIN | POLLRDNORM;
    return 0;
}

static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mycdrv_read,
    .poll = mycdrv_poll,
    .open = mycdrv_generic_open,
    .release = mycdrv_generic_release,
};

static inline int __init my_init (void)
{
    if (request_irq (irq, my_interrupt, IRQF_SHARED, "my_int", my_interrupt))
        return -1;

    atomic_set (&nevents, 0);
    printk (KERN_INFO "successfully loaded\n");
    return my_generic_init ();
}

static inline void __exit my_exit (void)
{
    synchronize_irq (irq);
    free_irq (irq, my_interrupt);
    printk (KERN_INFO "successfully unloaded\n");
    return my_generic_exit ();
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_17/lab4_interrupt.c");
MODULE_LICENSE ("GPL v2");
