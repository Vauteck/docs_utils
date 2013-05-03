/*
 * Includes
 */
#include <linux/kernel.h>	/* printk() */
#include <linux/module.h>	/* modules */
#include <linux/init.h>		/* module_{init,exit}() */
#include <linux/fs.h>

MODULE_DESCRIPTION("tp-char");
MODULE_AUTHOR("TG");
MODULE_LICENSE("GPL");

/*
 * Arguments
 */
static int major = 0; /* Major number */
module_param(major, int, 0660);
MODULE_PARM_DESC(major, "Static major number (none = dynamic)");

/*
 * File operations
 */
static ssize_t k_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	printk(KERN_DEBUG "char: read()\n");

	return count;
}

static ssize_t k_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	printk(KERN_DEBUG "char: write()\n");

	return count;
}

static int k_open(struct inode *inode, struct file *file)
{
	printk(KERN_DEBUG "char: open()\n");

	return 0;
}

static int k_release(struct inode *inode, struct file *file)
{
	printk(KERN_DEBUG "char: release()\n");

	return 0;
}

static struct file_operations k_fops = {
	.owner =	THIS_MODULE,
	.read =		k_read,
	.write =	k_write,
	.open =		k_open,
	.release =	k_release,
};

/*
 * Init and Exit
 */
static int __init k_init(void)
{
	int ret;

	ret = register_chrdev(major, "char", &k_fops);
	if (ret < 0) {
		printk(KERN_WARNING "char: unable to get a major\n");

		return ret;
	}

	if (major == 0)
		major = ret; /* dynamic value */

	printk(KERN_INFO "char: successfully loaded with major %d\n", major);

	return 0;
}

static void __exit k_exit(void)
{
	if (unregister_chrdev(major, "char") < 0) {
		printk(KERN_WARNING "char: error while unregistering\n");
		return;
	}

	printk(KERN_INFO "char: successfully unloaded\n");
}

/*
 * Module entry points
 */
module_init(k_init);
module_exit(k_exit);
