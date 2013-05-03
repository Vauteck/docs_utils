/* **************** LDD:1.0 s_20/lab7_eipc.c **************** */
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
 * Executing in Process Context
 *
 * Write a brief module that uses execute_in_process_context().  It
 * should do this first in process context (during intialization would
 * be sufficient) and then in an interrupt routine.
 *
 * You can adapt the simplest shared interrupt lab module to do this.
 *
 * Make sure you print out the return value in order to see whether it
 * just ran the function directly, or from a work queue.
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#define SHARED_IRQ 17
static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;
module_param (irq, int, S_IRUGO);

static struct execute_work ew;
static void tfun (struct work_struct *w)
{
    printk (KERN_INFO "I got into my function\n");
}

static irqreturn_t my_interrupt (int irq, void *dev_id)
{
    int rc;
    irq_counter++;
    printk (KERN_INFO "In the ISR: counter = %d\n", irq_counter);
    rc = execute_in_process_context (tfun, &ew);
    printk (KERN_INFO "rc from execute_in_process_context = %d\n", rc);
    return IRQ_NONE;            /* we return IRQ_NONE because we are just observing */
}

static int __init my_init (void)
{
    int rc;
    if (request_irq
        (irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id))
        return -1;
    printk (KERN_INFO "Successfully loading ISR handler\n");
    rc = execute_in_process_context (tfun, &ew);
    printk (KERN_INFO "rc from execute_in_process_context = %d\n", rc);

    return 0;
}

static void __exit my_exit (void)
{
    synchronize_irq (irq);
    free_irq (irq, &my_dev_id);
    printk (KERN_INFO "Successfully unloading,  irq_counter = %d\n",
            irq_counter);
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_20/lab7_eipc.c");
MODULE_LICENSE ("GPL v2");
