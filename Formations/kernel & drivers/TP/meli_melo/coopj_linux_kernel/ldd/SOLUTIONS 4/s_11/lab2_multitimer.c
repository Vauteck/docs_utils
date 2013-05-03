/* **************** "":1.0 s_11/lab2_multitimer.c **************** */
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
 * Multiple Kernel Timers
 *
 * Make the period in the first lab long enough so you can issue
 * multiple writes before the timer function run. (Hint: you may want
 * to save your data before running this lab.)
 *
 * How many times does the function get run?
 *
 * Fix the solution so multiple timers work properly.
 @*/

#include <linux/module.h>
#include "lab_char.h"
#include <linux/timer.h>
#include <linux/delay.h>

/* you probably don't need the ntimers variable, and it is a
   forward reference because we haven't done atomic variables
   but it is here to avoid unloading while there are still
   timers to unload.  It is also used sloppily on the exit :)
*/
static atomic_t ntimers;

struct my_dat {
	int l;
	struct timer_list *tl;
};

static void my_timer_function(unsigned long ptr)
{
	struct my_dat *tl = (struct my_dat *)ptr;
	printk(KERN_INFO "I am in my_timer_fun, jiffies = %ld\n", jiffies);
	printk(KERN_INFO " I think my current task pid is %d\n",
	       (int)current->pid);
	printk(KERN_INFO " my data is: %d\n", tl->l);
	kfree(tl->tl);
	kfree(tl);
	atomic_dec(&ntimers);
	printk(KERN_INFO "ntimers deced to %d\n", atomic_read(&ntimers));
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	struct timer_list *tl;
	struct my_dat *mdata;
	static int len = 100;
	atomic_inc(&ntimers);
	printk(KERN_INFO "ntimers upped to %d\n", atomic_read(&ntimers));
	tl = (struct timer_list *)kmalloc(sizeof(struct timer_list),
					  GFP_KERNEL);
	printk(KERN_INFO " Entering the WRITE function\n");
	printk(KERN_INFO " my current task pid is %d\n", (int)current->pid);
	init_timer(tl);		/* intialize */
	tl->function = my_timer_function;
	tl->expires = jiffies + 4 * HZ;	/* four second delay */
	mdata = (struct my_dat *)kmalloc(sizeof(struct my_dat), GFP_KERNEL);
	tl->data = (unsigned long)mdata;
	mdata->l = len;
	mdata->tl = tl;
	/*  my_timer.data = (void *) len; */

	printk(KERN_INFO "Adding timer at jiffies = %ld\n", jiffies);
	add_timer(tl);
	len += 100;
	return lbuf;
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_generic_read,
	.write = mycdrv_write,
	.open = mycdrv_generic_open,
	.release = mycdrv_generic_release,
};

static int __init my_init(void)
{
	atomic_set(&ntimers, 0);
	return my_generic_init();
}

static void __exit my_exit(void)
{
	/* wait for all timers to finish ; pretty crummy */
	printk(KERN_INFO "ntimers in remove routine to %d\n",
	       atomic_read(&ntimers));
	while (atomic_read(&ntimers)) {
		printk(KERN_INFO "sleeping, ntimers still %d\n",
		       atomic_read(&ntimers));
		msleep(1000);	/* wait a second, ugly */
	}
	my_generic_exit();
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION(""":1.0 s_11/lab2_multitimer.c");
MODULE_LICENSE("GPL v2");
