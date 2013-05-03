/* **************** "":1.0 s_18/mmapdrv.c **************** */
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
/* Sample Character Driver with mmap'ing */

#include <linux/module.h>	/* for modules */
#include <linux/fs.h>		/* file_operations */
#include <linux/uaccess.h>	/* copy_(to,from)_user */
#include <linux/init.h>		/* module_init, module_exit */
#include <linux/slab.h>		/* kmalloc */
#include <linux/cdev.h>		/* cdev utilities */
#include <linux/mm.h>		/* remap_pfn_range */

#define MYDEV_NAME "mycdrv"

static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
static struct cdev *my_cdev;

static int mycdrv_mmap(struct file *file, struct vm_area_struct *vma)
{
	printk(KERN_INFO "I entered the mmap function\n");
	if (remap_pfn_range(vma, vma->vm_start,
			    vma->vm_pgoff,
			    vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}

/* don't bother with open, release, read and write */

static struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.mmap = mycdrv_mmap,
};
static int __init my_init(void)
{
	first = MKDEV(my_major, my_minor);
	register_chrdev_region(first, count, MYDEV_NAME);
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &mycdrv_fops);
	cdev_add(my_cdev, first, count);
	printk(KERN_INFO "\nSucceeded in registering character device %s\n",
	       MYDEV_NAME);
	return 0;
}
static void __exit my_exit(void)
{
	cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	printk(KERN_INFO "\ndevice unregistered\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("Sample Memory Map Driver Entry");
MODULE_LICENSE("GPL v2");
