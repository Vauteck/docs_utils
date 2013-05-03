/* **************** LDD:1.0 s_17/lab1_semaphore_rw.c **************** */
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
 *: Using Wait Queues  (R/W semaphore solution)
 *
 *  Generalize the previous character driver to use wait queues,
 *
 *  Have the read() function go to sleep until woken by a write()
 *  function.  (You could also try reversing read and write.)
 *
 *  You may want to open up two windows and read in one window and
 *  then write in the other window.
 *
 *  Try putting more than one process to sleep, i.e., run your test
 *  read program more than once simultaneously before running the
 *  write program to awaken them.  If you keep track of the pid's you
 *  should be able to detect in what order processes are woken.
 *
 *  There are several solutions given:
 *
 *      Using wait_event_interruptible().  You may want to use atomic
 *      functions for any global variables used in the logical
 *      condition.
 *
 *      There are two solutions with this interface; one that wakes up
 *      only one sleeper, one that wakes up all sleepers.
 *
 *      Using wait_for_completion().
 *
 *      Using semaphores.
 *
 *      Using read/write semaphores.
 *
 *      Using exclusive waiting on the many readers solution..  How
 *      many processes wake up?
 *
 *  If you test with cat, echo, or dd, you may see different results
 *  than if you use the supplied simple read/write programs.  Why?
 @*/

#include <linux/module.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif
#include "lab_char.h"

static DECLARE_RWSEM (my_sem);

static ssize_t
mycdrv_read (struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
    printk (KERN_INFO "process %i (%s) going to sleep\n", current->pid,
            current->comm);
    down_read (&my_sem);
    printk (KERN_INFO "process %i (%s) awakening\n", current->pid,
            current->comm);
    return mycdrv_generic_read (file, buf, lbuf, ppos);
}

static ssize_t
mycdrv_write (struct file *file, const char __user * buf, size_t lbuf,
              loff_t * ppos)
{
    int nbytes = mycdrv_generic_write (file, buf, lbuf, ppos);

    printk (KERN_INFO "process %i (%s) awakening the readers...\n",
            current->pid, current->comm);
    up_read (&my_sem);
    return nbytes;
}
static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mycdrv_read,
    .write = mycdrv_write,
    .open = mycdrv_generic_open,
    .release = mycdrv_generic_release,
};

module_init (my_generic_init);
module_exit (my_generic_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_17/lab1_semaphore_rw.c");
MODULE_LICENSE ("GPL v2");
