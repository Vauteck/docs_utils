/* **************** LDD:1.0 s_14/lab2_proc.c **************** */
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

* Using the /proc filesystem. (/proc/driver solution)
*
* Write a module that creates a /proc filesystem entry and can read
* and write to it.
*
* When you read from the entry, you should obtain the value of some
* parameter set in your module.
*
* When you write to the entry, you should modify that value, which
* should then be reflected in a subsequent read.
*
* Make sure you remove the entry when you unload your module.  What
* happens if you don't and you try to access the entry after the
* module has been removed?
*
* There are two different solutions given, one which creates the entry
* in the /proc directory, the other in /proc/driver.
@*/

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>

#if 0
#define NODE "my_proc"
#else
#define NODE "driver/my_proc"
#endif

static int param = 100;
static struct proc_dir_entry *my_proc;

static int
my_proc_read (char *page, char **start, off_t off, int count,
              int *eof, void *data)
{
    *eof = 1;
    return sprintf (page, "%d\n", param);
}

static int
my_proc_write (struct file *file, const char __user * buffer,
               unsigned long count, void *data)
{
    char *str;
    str = kmalloc ((size_t) count, GFP_KERNEL);
    if (copy_from_user (str, buffer, count)) {
        kfree (str);
        return -EFAULT;
    }
    sscanf (str, "%d", &param);
    printk (KERN_INFO "param has been set to %d\n", param);
    kfree (str);
    return count;
}

static int __init my_init (void)
{
    my_proc = create_proc_entry (NODE, S_IRUGO | S_IWUSR, NULL);
    if (!my_proc) {
        printk (KERN_ERR "I failed to make %s\n", NODE);
        return -1;
    }
    printk (KERN_INFO "I created %s\n", NODE);
    my_proc->read_proc = my_proc_read;
    my_proc->write_proc = my_proc_write;
    return 0;
}

static void __exit my_exit (void)
{
    if (my_proc) {
        remove_proc_entry (NODE, NULL);
        printk (KERN_INFO "Removed %s\n", NODE);
    }
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_14/lab2_proc.c");
MODULE_LICENSE ("GPL v2");
