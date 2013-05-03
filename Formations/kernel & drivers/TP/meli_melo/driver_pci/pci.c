/*
 * Includes
 */
#include <linux/kernel.h>	/* printk() */
#include <linux/module.h>	/* modules */
#include <linux/init.h>		/* module_{init,exit}() */
#include <linux/slab.h>		/* kmalloc()/kfree() */
#include <linux/pci.h>		/* pci_*() */
#include <linux/pci_ids.h>	/* pci idents */
#include <linux/list.h>		/* list_*() */
#include <asm/uaccess.h>	/* copy_{from,to}_user() */
#include <linux/fs.h>		/* file_operations */
#include <linux/interrupt.h>	/* request_irq etc */
#include <linux/version.h>

MODULE_DESCRIPTION("tp-pci");
MODULE_AUTHOR("TG");
MODULE_LICENSE("GPL");

/*
 * Arguments
 */
static int major = 0; /* Major number */
module_param(major, int, 0660);
MODULE_PARM_DESC(major, "Static major number (none = dynamic)");

/*
 * Supported devices
 */
static struct pci_device_id k_id_table[] __devinitdata = {
	{PCI_VENDOR_ID_INTEL, 0x2572, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{PCI_VENDOR_ID_INTEL, 0x1050, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0,}	/* 0 terminated list */
};
MODULE_DEVICE_TABLE(pci, k_id_table);

/*
 * Global variables
 */
static LIST_HEAD(k_list);

struct k_struct {
	struct list_head	link; /* Double linked list */
	struct pci_dev		*dev; /* PCI device */
	int			minor; /* Minor number */
	void			*mmio[DEVICE_COUNT_RESOURCE]; /* Remaped I/O memory */
	u32			mmio_len[DEVICE_COUNT_RESOURCE]; /* Size of remaped I/O memory */
};

/*
 * Event handlers
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
irqreturn_t k_irq_handler(int irq, void *dev_id)
#else
irqreturn_t k_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
	struct k_struct *data = (struct k_struct *)dev_id;
	//struct pci_dev *dev = data->dev;

	printk(KERN_DEBUG "pci: interrupt from device %d\n", data->minor);

	return IRQ_NONE;
}

/*
 * File operations
 */
static ssize_t k_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	int i, real, bank = DEVICE_COUNT_RESOURCE;
	struct k_struct *data = file->private_data;

	/* Find the first remapped I/O memory bank to read */
	for (i = 0; i < DEVICE_COUNT_RESOURCE; i++) {
		if (data->mmio[i] != NULL) {
			bank = i;

			break;
		}
	}

	/* No bank found */
	if (bank == DEVICE_COUNT_RESOURCE) {
		printk(KERN_WARNING "pci: no remaped I/O memory bank to read\n");

		return -ENXIO;
	}

	/* Check for overflow */
	real = min(data->mmio_len[bank] - (int)*ppos, count);

	if (real)
		if (copy_to_user(buf, (char*)data->mmio[bank] + (int)*ppos, real))
			return -EFAULT;

	printk(KERN_DEBUG "pci: read %d/%d chars at offset %d from remaped I/O memory bank %d\n", real, count, (int)*ppos, bank);

	*ppos += real;

	return real;
}

static int k_open(struct inode *inode, struct file *file)
{
	int minor = MINOR(inode->i_rdev);
	struct list_head *cur;
	struct k_struct *data;

	printk(KERN_DEBUG "k_open()\n");

	list_for_each(cur, &k_list) {
		data = list_entry(cur, struct k_struct, link);

		if (data->minor == minor) {
			file->private_data = data;

			return 0;
		}
	}

	printk(KERN_WARNING "pci: minor %d not found\n", minor);

	return -ENODEV;
}

static int k_release(struct inode *inode, struct file *file)
{
	printk(KERN_DEBUG "k_release()\n");

	file->private_data = NULL;

	return 0;
}

static struct file_operations k_fops = {
	.owner =	THIS_MODULE,
	.read =		k_read,
	.open =		k_open,
	.release =	k_release,
};

/*
 * PCI handling
 */
static int __devinit k_probe(struct pci_dev *dev, const struct pci_device_id *ent)
{
	int i, ret = 0;
	struct k_struct *data;
	static int minor = 0;

	printk(KERN_INFO "pci: found %x:%x\n", ent->vendor, ent->device);
	printk(KERN_INFO "pci: using major %d and minor %d for this device\n", major, minor);

	/* Allocate a private structure and reference it as driver's data */
	data = (struct k_struct *)kmalloc(sizeof(struct k_struct), GFP_KERNEL);
	if (data == NULL) {
		printk(KERN_WARNING "pci: unable to allocate private structure\n");

		ret = -ENOMEM;
		goto cleanup_kmalloc;
	}

	pci_set_drvdata(dev, data);

	/* Init private field */
	data->dev = dev;
	data->minor = minor++;

 	/* Initialize device before it's used by the driver */
	ret = pci_enable_device(dev);
	if (ret < 0) {
		printk(KERN_WARNING "pci: unable to initialize PCI device\n");

		goto cleanup_pci_enable;
	}

	/* Reserve PCI I/O and memory resources */
	ret = pci_request_regions(dev, "pci");
	if (ret < 0) {
		printk(KERN_WARNING "pci: unable to reserve PCI resources\n");

		goto cleanup_regions;
	}

	/* Inspect PCI BARs and remap I/O memory */
	for (i=0; i < DEVICE_COUNT_RESOURCE; i++) {
		if (pci_resource_len(dev, i) == 0)
			continue;

		if (pci_resource_start(dev, i) == 0)
			continue;

		printk(KERN_DEBUG "pci: BAR %d (%#08x-%#08x), len=%d, flags=%#08x\n", i, (u32) pci_resource_start(dev, i), (u32) pci_resource_end(dev, i), (u32) pci_resource_len(dev, i), (u32) pci_resource_flags(dev, i));

		if (pci_resource_flags(dev, i) & IORESOURCE_MEM) {
			data->mmio[i] = ioremap(pci_resource_start(dev, i), pci_resource_len(dev, i));
			if (data->mmio[i] == NULL) {
				printk(KERN_WARNING "pci: unable to remap I/O memory\n");

				ret = -ENOMEM;
				goto cleanup_ioremap;
			}

			data->mmio_len[i] = pci_resource_len(dev, i);

			printk(KERN_DEBUG "pci: I/O memory has been remaped at %#08x\n", (u32)data->mmio[i]);
		} else {
			data->mmio[i] = NULL;
		}
	}

	/* Install the irq handler */
	ret = request_irq(dev->irq, k_irq_handler, SA_SHIRQ, "pci", data);
	if (ret < 0) {
		printk(KERN_WARNING "pci: unable to register irq handler\n");

		goto cleanup_irq;
	}

 	/* Enable PCI bus-mastering for this device */
	pci_set_master(dev);

	/* Link the new data structure with others */
	list_add_tail(&data->link, &k_list);

	return 0;

cleanup_irq:
	for (i--; i >= 0; i--) {
		if (data->mmio[i] != NULL)
			iounmap(data->mmio[i]);
	}
cleanup_ioremap:
	pci_release_regions(dev);
cleanup_regions:
	pci_disable_device(dev);
cleanup_pci_enable:
	kfree(data);
cleanup_kmalloc:
	return ret;
}

static void __devexit k_remove(struct pci_dev *dev)
{
	int i;
	struct k_struct *data = pci_get_drvdata(dev);

	printk(KERN_INFO "pci: device removed\n");

	for (i=0; i < DEVICE_COUNT_RESOURCE; i++) {
		if (data->mmio[i] != NULL)
			iounmap(data->mmio[i]);
	}

	pci_release_regions(dev);

	pci_disable_device(dev);

	free_irq(dev->irq, data);

	list_del(&data->link);

	kfree(data);
}

static struct pci_driver k_driver = {
	.name =		"pci",
	.id_table =	k_id_table,
	.probe =	k_probe,		/* Init one device */
	.remove =	k_remove,		/* Remove one device */
};

/*
 * Init and Exit
 */
static int __init k_init(void)
{
	int ret;

	/* Register the device driver */
	ret = register_chrdev(major, "pci", &k_fops);
	if (ret < 0) {
		printk(KERN_WARNING "pci: unable to get a major\n");

		return ret;
	}

	if (major == 0)
		major = ret; /* dynamic value */

	/* Register PCI driver */
	ret = pci_register_driver(&k_driver);
	if (ret < 0) {
		printk(KERN_WARNING "pci: unable to register PCI driver\n");

		unregister_chrdev(major, "pci");

		return ret;
	}

	return 0;
}

static void __exit k_exit(void)
{
	pci_unregister_driver(&k_driver);

	unregister_chrdev(major, "pci");
}

/*
 * Module entry points
 */
module_init(k_init);
module_exit(k_exit);
