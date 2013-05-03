/* **************** LDD:1.0 s_16/lab4_relay_module.c **************** */
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
 * Using Relay Channels. (Kernel Module)
 *
 * Write a kenel module that opens up a relay channel and makes the
 * associated files visible in the debugfs filesystem.
 *
 * Make sure you mount the filesystem (if necessary) with
 *
 *     mount -t debugfs none /sys/kernel/debug
 *
 * Have the initialization routine write a series of entries into the
 * channel. While the kernel module is loaded, try reading from it
 * using read() and mmap().
 *
 * If you read more than once on the open file descriptor what do you
 * see?
 *
 * For more advanced exercises, you might try making sure your kernel
 * client writes over sub-buffer boundaries, or writes into the
 * channel from other functions such as an interrupt routine, or other
 * entry points.
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/relay.h>
#include <linux/debugfs.h>

/* 16 page sized sub-buffers, total buffer size = 16 pages */

#define SB_SIZE PAGE_SIZE
#define N_SB 16

static struct rchan *mychan;
static char *fname = "my_rc_file";

static struct dentry
    *create_buf_file_handler (const char *filename,
                              struct dentry *parent,
                              int mode, struct rchan_buf *buf, int *is_global)
{
    //    *is_global=1; /* turn on if you want only one file on SMP */
    return debugfs_create_file (filename, mode, parent, buf,
                                &relay_file_operations);
}

static int remove_buf_file_handler (struct dentry *dentry)
{
    debugfs_remove (dentry);
    return 0;
}

static struct rchan_callbacks relay_callbacks = {
    .create_buf_file = create_buf_file_handler,
    .remove_buf_file = remove_buf_file_handler,
};

static int __init my_init (void)
{
    int j, k;
    char data[64];
    mychan = relay_open (fname, NULL, SB_SIZE, N_SB, &relay_callbacks, NULL);

    /* can't use smp_processor_id() properly otherwise */
    for (k = 0; k < 10; k++) {  /* do this for a minute */
        preempt_disable ();
        for (j = 0; j < 10; j++) {
            memset (data, 0, 64);
            sprintf (data, "data=%d\n", j * k);
            relay_write (mychan, data, 64);
        }
        printk (KERN_INFO "Relay Channel %s loaded on cpu=%d.\n", fname,
                smp_processor_id ());
        preempt_enable ();
        msleep (6000);
    }
    return 0;
}

static void __exit my_exit (void)
{
    relay_close (mychan);
    printk (KERN_INFO "Relay Channel %s unloaded.\n", fname);
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_16/lab4_relay_module.c");
MODULE_LICENSE ("GPL v2");
