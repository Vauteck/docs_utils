/*
 * Includes
 */
#include <linux/kernel.h>	/* printk() */
#include <linux/module.h>	/* modules */
#include <linux/init.h>		/* module_{init,exit}() */
#include <linux/slab.h>		/* kmalloc()/kfree() */
#include <asm/uaccess.h>	/* copy_{from,to}_user() */
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

static size_t buf_size = 64; /* Buffer size */
module_param(buf_size, int, 0660);
MODULE_PARM_DESC(buf_size, "Buffer size");

/*
 * Global variables
 */
static char *buffer; /* The buffer */
static size_t num = 0; /* Number of available bytes in the buffer */

/*
 * File operations
 */
static ssize_t k_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	size_t real;

	real = min(num, count);

	if (real)
		if (copy_to_user(buf, buffer, real))
			return -EFAULT;

	num = 0; /* Destructive read (no more data after a read) */

	printk(KERN_DEBUG "char: read %d/%d chars\n", real, count);

	return real;
}

static ssize_t k_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	size_t real;

	real = min(buf_size, count);

	if (real)
		if (copy_from_user(buffer, buf, real))
			return -EFAULT;

	num = real; /* Destructive write (overwrite previous data if any) */

	printk(KERN_DEBUG "char: wrote %d/%d chars\n", real, count);

	return real;
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

	buffer = (char *)kmalloc(buf_size, GFP_KERNEL);
	if (buffer != NULL) {
		printk(KERN_DEBUG "char: allocated a %d bytes buffer\n", buf_size);
	} else {
		printk(KERN_WARNING "char: unable to allocate a %d bytes buffer\n", buf_size);

		unregister_chrdev(major, "char");

		return -ENOMEM;
	}

	printk(KERN_INFO "char: successfully loaded with major %d\n", major);

	return 0;
}

static void __exit k_exit(void)
{
	kfree(buffer);

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
