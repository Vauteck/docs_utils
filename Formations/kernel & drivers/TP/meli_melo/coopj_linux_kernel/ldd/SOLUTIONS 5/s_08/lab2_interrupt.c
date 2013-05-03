/* **************** CLDD:1.0 s_08/lab2_interrupt.c **************** */
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
 * Sharing All Interrupts  (Kernel Module)
 *
 * Extend the previous solution to construct a character driver that
 * shares every possible interrupt with already installed handlers.
 *
 * The highest interrupt number you have to consider will depend on
 * your kernel and platform; look at /proc/interrupts to ascertan what
 * is necessary.
 *
 * Take particular care when you call free_irq() as it is very easy to
 * freeze your system if you are not careful.
 *
 * The character driver can be very simple; for instance if no open()
 * and release() methods are specified, success is the default.
 *
 * A read() on the device should return a brief report on the total
 * number of interrupts handled for each IRQ.
 *
 * To do this you'll also have to write a short application to
 * retrieve and print out the data.  (Don't forget to create the
 * device node before you run the application.)
 *
 @*/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>

#define MYDEV_NAME "mycdrv"
#define KBUF_SIZE (size_t)(PAGE_SIZE)
#define MAXIRQS 256
#define NCOPY (MAXIRQS * sizeof(int))

static char *kbuf;
static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
static struct cdev *my_cdev;

static int *interrupts;

static irqreturn_t my_interrupt (int irq, void *dev_id)
{
    interrupts[irq]++;
    return IRQ_NONE;            /* we return IRQ_NONE because we are just observing */
}
static void freeup_irqs (void)
{
    int irq;
    for (irq = 0; irq < MAXIRQS; irq++) {
        if (interrupts[irq] >= 0) { /* if greater than 0, was able to share */
            synchronize_irq (irq);
            printk (KERN_INFO "Freeing IRQ= %4d, which had %10d events\n",
                    irq, interrupts[irq]);
            free_irq (irq, interrupts);
        }
    }
}
static ssize_t
mycdrv_read (struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
    int nbytes, maxbytes, bytes_to_do;
    maxbytes = KBUF_SIZE - *ppos;
    bytes_to_do = maxbytes > lbuf ? lbuf : maxbytes;
    if (bytes_to_do == 0)
        printk (KERN_INFO "Reached end of the device on a read");
    nbytes = bytes_to_do - copy_to_user (buf, kbuf + *ppos, bytes_to_do);
    *ppos += nbytes;
    printk (KERN_INFO "\n Leaving the   READ function, nbytes=%d, pos=%d\n",
            nbytes, (int)*ppos);
    return nbytes;
}

static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mycdrv_read,
};

static int __init my_init (void)
{
    int irq;

    first = MKDEV (my_major, my_minor);

    if (register_chrdev_region (first, count, MYDEV_NAME) < 0) {
        printk (KERN_ERR "failed to register character device region\n");
        return -1;
    }
    if (!(my_cdev = cdev_alloc ())) {
        printk (KERN_ERR "cdev_alloc() failed\n");
        unregister_chrdev_region (first, count);
        return -1;
    }
    cdev_init (my_cdev, &mycdrv_fops);
    kbuf = kmalloc (KBUF_SIZE, GFP_KERNEL);
    if (cdev_add (my_cdev, first, count) < 0) {
        printk (KERN_ERR "cdev_add() failed\n");
        cdev_del (my_cdev);
        unregister_chrdev_region (first, count);
        kfree (kbuf);
        return -1;
    }

    printk (KERN_INFO "\nSucceeded in registering character device %s\n",
            MYDEV_NAME);
    printk (KERN_INFO "Major number = %d, Minor number = %d\n", MAJOR (first),
            MINOR (first));

    interrupts = (int *)kbuf;

    for (irq = 0; irq < MAXIRQS; irq++) {
        interrupts[irq] = -1;   /* set to -1 as a flag */
        if (!request_irq (irq, my_interrupt, IRQF_SHARED, "my_int", interrupts)) {
            interrupts[irq] = 0;
            printk (KERN_INFO "Succeded in registering IRQ=%d\n", irq);
        }
    }

    return 0;
}

static void __exit my_exit (void)
{
    freeup_irqs ();
    cdev_del (my_cdev);
    unregister_chrdev_region (first, count);
    kfree (kbuf);
    printk (KERN_INFO "\ndevice unregistered\n");
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("CLDD:1.0 s_08/lab2_interrupt.c");
MODULE_LICENSE ("GPL v2");
