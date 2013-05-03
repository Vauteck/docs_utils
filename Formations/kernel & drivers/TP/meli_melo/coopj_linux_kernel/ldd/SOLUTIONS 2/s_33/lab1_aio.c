/* **************** LDD:1.0 s_33/lab1_aio.c **************** */
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
 * Adding Asynchronous Entry Points to a Character Driver
 *
 *   (kernel driver)
 *
 * Take one of your earlier character drivers and add new entry points
 * for aio_read() and aio_write().
 *
 * To test this you'll need to write a user-space program that uses
 * the native Linux API.  Have it send out a number of write and read
 * requests and synchronize properly.
 *
 * We also present a solution using the Posix API for the user
 * application; note that this will never hit your driver unless you
 * comment out the normal read and write entry points in which case
 * the kernel will fall back on the asynchronous ones.
 *
 * Make sure you compile by linking with the right libraries; use
 * -laio for the Linux API and -lrt for the Posix API.  (You can use
 * both in either case as they don't conflict.)'
 @*/

#include <linux/module.h>
#include "lab_char.h"

#include <linux/aio.h>          /* for aio stuff */

static ssize_t mycdrv_aio_read (struct kiocb *iocb, const struct iovec *iov,
                                unsigned long niov, loff_t offset)
{
    printk (KERN_INFO "entering mycdrv_aio_read\n");
    return mycdrv_generic_read (iocb->ki_filp, iov->iov_base, iov->iov_len,
                                &offset);
}

static ssize_t mycdrv_aio_write (struct kiocb *iocb, const struct iovec *iov,
                                 unsigned long niov, loff_t offset)
{
    printk (KERN_INFO "entering mycdrv_aio_write\n");
    return mycdrv_generic_write (iocb->ki_filp, iov->iov_base, iov->iov_len,
                                 &offset);
}

static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mycdrv_generic_read,
    .write = mycdrv_generic_write,
    .open = mycdrv_generic_open,
    .release = mycdrv_generic_release,
    .llseek = mycdrv_generic_lseek,
    .aio_read = mycdrv_aio_read,
    .aio_write = mycdrv_aio_write,
};

module_init (my_generic_init);
module_exit (my_generic_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_33/lab1_aio.c");
MODULE_LICENSE ("GPL v2");
