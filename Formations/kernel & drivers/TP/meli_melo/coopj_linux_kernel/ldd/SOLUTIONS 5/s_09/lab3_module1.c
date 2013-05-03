/* **************** CLDD:1.0 s_09/lab3_module1.c **************** */
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
 * Dynamic Module Loading  (module 1)
 *
 * Take your basic character driver from the previous exercise and
 * adapt it to use dynamic loading:
 *
 * Construct a trivial second module and have it dynamically loaded
 * during the character driver's open entry point. (Make sure the name
 * of the file that is requested is the same as the name of your
 * file.)
 *
 * Add a small function to your character driver and have it
 * referenced by the second module.
 *
 * Make sure you place your modules in a place where modprobe can find
 * them, (Installing with the target modules_install and then running
 * depmod -ae will take care of this for you.)
 *
 * Make sure you place your modules in a place where modprobe can find
 * them.  (Installing with the target modules_install will take care
 * of this for you.)
 *
 * You can use either cat or the main program from the character
 * driver lab to exercise your module.  What happens if you try to
 * request loading more than once?
 *
 @*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>

#define MYDEV_NAME "mycdrv"
#define KBUF_SIZE (size_t)(10*PAGE_SIZE)

static char *kbuf;
static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
static struct cdev *my_cdev;

void mod_fun (void)
{
    printk (KERN_INFO " VOILA! I got into mod_fun \n");
}

EXPORT_SYMBOL (mod_fun);

static int mycdrv_open (struct inode *inode, struct file *file)
{
    static int counter = 0;
    printk (KERN_INFO " attempting to open device: %s:\n", MYDEV_NAME);
    printk (KERN_INFO " MAJOR number = %d, MINOR number = %d\n",
            imajor (inode), iminor (inode));
    counter++;

    printk (KERN_INFO " successfully open  device: %s:\n\n", MYDEV_NAME);
    printk (KERN_INFO "I have been opened  %d times since being loaded\n",
            counter);
    printk (KERN_INFO "ref=%d\n", module_refcount (THIS_MODULE));

    printk (KERN_INFO "/n rc from requesting module mod_fun is: %d\n",
            request_module ("%s", "lab1_module2"));

    return 0;
}

static int mycdrv_release (struct inode *inode, struct file *file)
{
    printk (KERN_INFO " CLOSING device: %s:\n\n", MYDEV_NAME);
    return 0;
}

static ssize_t
mycdrv_read (struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
    int nbytes = lbuf - copy_to_user (buf, kbuf + *ppos, lbuf);
    *ppos += nbytes;
    printk (KERN_INFO "\n READING function, nbytes=%d, pos=%d\n", nbytes,
            (int)*ppos);
    return nbytes;
}

static ssize_t
mycdrv_write (struct file *file, const char __user * buf, size_t lbuf,
              loff_t * ppos)
{
    int nbytes = lbuf - copy_from_user (kbuf + *ppos, buf, lbuf);
    *ppos += nbytes;
    printk (KERN_INFO "\n WRITING function, nbytes=%d, pos=%d\n", nbytes,
            (int)*ppos);
    return nbytes;
}

static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mycdrv_read,
    .write = mycdrv_write,
    .open = mycdrv_open,
    .release = mycdrv_release,
};

static int __init my_init (void)
{
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
    return 0;
}

static void __exit my_exit (void)
{
    if (my_cdev)
        cdev_del (my_cdev);
    unregister_chrdev_region (first, count);
    if (kbuf)
        kfree (kbuf);
    printk (KERN_INFO "\ndevice unregistered\n");
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("CLDD:1.0 s_09/lab3_module1.c");
MODULE_LICENSE ("GPL v2");
