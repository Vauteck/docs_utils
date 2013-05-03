/* **************** "":1.0 s_20/lab_all_interrupt.h **************** */
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
#ifndef _LAB_ALL_INTERRUPT_H
#define _LAB_ALL_INTERRUPT_H

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/device.h>

#define MYDEV_NAME "mycdrv"
#define ramdisk_size (size_t)(PAGE_SIZE)
#define MAXIRQS 256
#define NCOPY (MAXIRQS * sizeof(atomic_t))

static char *ramdisk;
static dev_t first;
static unsigned int count = 1;
static struct cdev *my_cdev;
static struct class *foo_class;

static atomic_t *interrupts, *bhs, *nevents;

/* default delay time in top half -- try 10 to get results */
static int delay = 0;
module_param(delay, int, S_IRUGO);

static irqreturn_t my_interrupt(int irq, void *dev_id);
#ifdef THREADED_IRQ
static irqreturn_t thread_fun(int irq, void *thr_arg);
#endif

static void freeup_irqs(void)
{
	int irq;
	for (irq = 0; irq < MAXIRQS; irq++) {
		/* if greater than 0, was able to share */
		if (atomic_read(&interrupts[irq]) >= 0) {
			printk
			    (KERN_INFO
			     "Freeing IRQ= %4d, which had %10d, %10d events\n",
			     irq, atomic_read(&interrupts[irq]),
			     atomic_read(&bhs[irq]));
			synchronize_irq(irq);
			free_irq(irq, interrupts);
		}
	}
}
static void get_irqs(void)
{
	int irq;
	interrupts = (atomic_t *) ramdisk;
	bhs = (atomic_t *) (ramdisk + NCOPY);

	for (irq = 0; irq < MAXIRQS; irq++) {
		atomic_set(&interrupts[irq], -1);	/* set to -1 as a flag */
		atomic_set(&bhs[irq], 0);
		atomic_set(&nevents[irq], 0);
#ifdef THREADED_IRQ
		if (!request_threaded_irq(irq, my_interrupt, thread_fun,
					  IRQF_SHARED, "my_int", interrupts))
#else
		if (!request_irq(irq, my_interrupt,
				 IRQF_SHARED, "my_int", interrupts))
#endif
		{
			atomic_set(&interrupts[irq], 0);
			printk(KERN_INFO "Succeded in registering IRQ=%d\n",
			       irq);
		}
	}
}

static ssize_t
mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes, maxbytes, bytes_to_do;
	maxbytes = ramdisk_size - *ppos;
	bytes_to_do = maxbytes > lbuf ? lbuf : maxbytes;
	if (bytes_to_do == 0)
		printk(KERN_INFO "Reached end of the device on a read");
	nbytes = bytes_to_do - copy_to_user(buf, ramdisk + *ppos, bytes_to_do);
	*ppos += nbytes;
	printk(KERN_INFO "\n Leaving the   READ function, nbytes=%d, pos=%d\n",
	       nbytes, (int)*ppos);
	return nbytes;
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_read,
};

static int __init my_generic_init(void)
{
	if (alloc_chrdev_region(&first, 0, count, MYDEV_NAME) < 0) {
		printk(KERN_ERR "failed to allocate character device region\n");
		return -1;
	}

	if (!(my_cdev = cdev_alloc())) {
		printk(KERN_ERR "cdev_alloc() failed\n");
		unregister_chrdev_region(first, count);
		return -1;
	}
	nevents = kmalloc(MAXIRQS * sizeof(atomic_t), GFP_KERNEL);
	ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
	cdev_init(my_cdev, &mycdrv_fops);
	if (cdev_add(my_cdev, first, count) < 0) {
		printk(KERN_ERR "cdev_add() failed\n");
		cdev_del(my_cdev);
		unregister_chrdev_region(first, count);
		kfree(ramdisk);
		kfree(nevents);
		return -1;
	}

	foo_class = class_create(THIS_MODULE, "my_class");
	device_create(foo_class, NULL, first, NULL, "%s", "mycdrv");

	printk(KERN_INFO "\nSucceeded in registering character device %s\n",
	       MYDEV_NAME);
	printk(KERN_INFO "Major number = %d, Minor number = %d\n", MAJOR(first),
	       MINOR(first));
	get_irqs();

	return 0;
}

static void __exit my_generic_exit(void)
{
	device_destroy(foo_class, first);
	class_destroy(foo_class);
	freeup_irqs();
	cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	kfree(ramdisk);
	kfree(nevents);
	printk(KERN_INFO "\ndevice unregistered\n");
}

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION(""":1.0 s_20/lab_all_interrupt.h");
MODULE_LICENSE("GPL v2");

#endif
