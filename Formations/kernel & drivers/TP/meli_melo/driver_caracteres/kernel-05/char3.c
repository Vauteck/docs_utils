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

static int max_devices = 2; /* Maximum supported devices */
module_param(max_devices, int, 0660);
MODULE_PARM_DESC(max_devices, "Maximum supported devices");

/*
 * Global variables
 */
static struct k_struct {
	char *buffer; /* The buffer */
	size_t num; /* Number of available bytes in the buffer */
} *k_tab;

/*
 * File operations
 */
static ssize_t k_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	size_t real;
	int minor = MINOR(file->f_dentry->d_inode->i_rdev);

	real = min(k_tab[minor].num, count);

	if (real)
		if (copy_to_user(buf, k_tab[minor].buffer, real))
			return -EFAULT;

	k_tab[minor].num = 0; /* Destructive read (no more data after a read) */

	printk(KERN_DEBUG "char: read %d/%d chars\n", real, count);

	return real;
}

static ssize_t k_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	size_t real;
	int minor = MINOR(file->f_dentry->d_inode->i_rdev);

	real = min(buf_size, count);

	if (real)
		if (copy_from_user(k_tab[minor].buffer, buf, real))
			return -EFAULT;

	k_tab[minor].num = real; /* Destructive write (overwrite previous data if any) */

	printk(KERN_DEBUG "char: wrote %d/%d chars\n", real, count);

	return real;
}

static int k_open(struct inode *inode, struct file *file)
{
	int minor = MINOR(inode->i_rdev);

	if (minor >= max_devices)
		return -ENODEV;

	printk(KERN_DEBUG "char: open()\n");

	return 0;
}

static int k_release(struct inode *inode, struct file *file)
{
	int minor = MINOR(inode->i_rdev);

	if (minor >= max_devices)
		return -ENODEV;

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
	int i, j, ret;

	/* Register the device driver */
	ret = register_chrdev(major, "char", &k_fops);
	if (ret < 0) {
		printk(KERN_WARNING "char: unable to get a major\n");
		return ret;
	}

	if (major == 0)
		major = ret; /* dynamic value */

	/* Allocate private structures */
	k_tab = (struct k_struct *)kmalloc(max_devices * sizeof(struct k_struct), GFP_KERNEL);
	if (k_tab != NULL) {
		printk(KERN_DEBUG "char: allocated a %dx%d bytes for private structures\n", max_devices, sizeof(struct k_struct));
	} else {
		printk(KERN_DEBUG "char: unable to allocate a %dx%d bytes for private structures\n", max_devices, sizeof(struct k_struct));

		unregister_chrdev(major, "char");

		return -ENOMEM;
	}

	/* Setup private structures */
	for (i = 0; i < max_devices; i++) {

		/* Allocate the buffer */
		k_tab[i].buffer = (char *)kmalloc(buf_size, GFP_KERNEL);
		if (k_tab[i].buffer != NULL) {
			printk(KERN_DEBUG "char: allocated a %d bytes buffer for device %d\n", buf_size, i);
		} else {
			printk(KERN_WARNING "char: unable to allocate a %d bytes buffer for device %d\n", buf_size, i);

			i--; /* Current index doesn't need cleanup */
			ret = -ENOMEM;
			goto cleanup;
		}

		/* Init values */
		k_tab[i].num = 0;
	}

	printk(KERN_INFO "char: successfully loaded with major %d\n", major);

	return 0;

cleanup:
	for (j = i; j >= 0; j--)
		kfree(k_tab[j].buffer);

	kfree(k_tab);

	unregister_chrdev(major, "char");

	return ret;
}

static void __exit k_exit(void)
{
	int i;

	for (i = 0; i < max_devices; i++)
		kfree(k_tab[i].buffer);

	kfree(k_tab);

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
