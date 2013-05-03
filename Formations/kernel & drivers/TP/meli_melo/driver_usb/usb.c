#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <asm/uaccess.h>
#include <linux/usb.h>
#include <linux/version.h>

MODULE_DESCRIPTION("tp-usb");
MODULE_AUTHOR("TG");
MODULE_LICENSE("GPL");

#define USB_VENDOR_ID	0x15ca
#define USB_PRODUCT_ID	0x00c3

#define USB_MINOR_BASE	42

/*
 * Supported devices
 */
static struct usb_device_id k_id_table[] __devinitdata = {
	{ USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
	{ }					/* Terminating list */
};
MODULE_DEVICE_TABLE(usb, k_id_table);

struct k_struct {
	struct usb_device *	udev;			/* the usb device for this device */
	struct usb_interface *	interface;		/* the interface for this device */
	__u8			int_in_endpointAddr;	/* the address of the interrupt endpoint */
	__le16			int_in_size;		/* max size of the transfer */
	__u8			int_in_interval;	/* polling interval */
	struct urb *		urb;			/* usb request block */
	unsigned char *		buffer;			/* transferred data */
	struct kref		kref;
};

/* forward declaration */
static struct usb_driver k_driver;

#define to_k_struct(d) container_of(d, struct k_struct, kref)

static void k_delete(struct kref *kref)
{
        struct k_struct *data = to_k_struct(kref);

	if (data->urb) {
		usb_kill_urb(data->urb);
		if (data->buffer)
			usb_buffer_free(data->udev, data->int_in_size,
					data->buffer, data->urb->transfer_dma);
		usb_free_urb(data->urb);
	}

        usb_put_dev(data->udev);
        kfree(data);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
static void k_complete(struct urb* urb)
#else
static void k_complete(struct urb* urb, struct pt_regs *regs)
#endif
{
	int i;
	unsigned char *p = urb->transfer_buffer;

        if (urb->status)
		return;

	for (i = 0; i < urb->actual_length; i++)
		printk(" %02x", p[i]);
	printk("\n");

	usb_submit_urb(urb, GFP_ATOMIC);
}

/*
 * File operations
 */
static ssize_t k_read(struct file *file, char *buffer, size_t count, loff_t *ppos)
{
	return count;
}

static int k_open(struct inode *inode, struct file *file)
{
	struct k_struct *data;
	struct usb_interface *interface;
	int minor = MINOR(inode->i_rdev);

	printk(KERN_DEBUG "k_open()\n");

	interface = usb_find_interface(&k_driver, minor);
	if (!interface) {
		printk(KERN_WARNING "usb: cannot find device for minor\n");
		return -ENODEV;
	}

	data = usb_get_intfdata(interface);
	if (!data)
		return -ENODEV;

	/* increment our usage count for the device */
	kref_get(&data->kref);

	file->private_data = data;

	if (usb_submit_urb(data->urb, GFP_ATOMIC)) {
		kref_put(&data->kref, k_delete);
		return -EIO;
	}

	return 0;
}

static int k_release(struct inode *inode, struct file *file)
{
	struct k_struct *data;

	printk(KERN_DEBUG "k_release()\n");

	data = file->private_data;
	if (data == NULL)
		return -ENODEV;

	usb_kill_urb(data->urb);

	/* decrement the count on our device */
	kref_put(&data->kref, k_delete);

	file->private_data = NULL;

	return 0;
}

static struct file_operations k_fops = {
	.owner =	THIS_MODULE,
	.open =		k_open,
	.read =		k_read,
	.release =	k_release,
};

static struct usb_class_driver k_class = {
	.name =		"usb/k_usb%d",
	.fops =		&k_fops,
	.minor_base = 	USB_MINOR_BASE,
};

static int __devinit k_probe(struct usb_interface *interface, const struct usb_device_id *ent)
{
	int i, ret = 0;
	struct k_struct *data;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;

	printk(KERN_INFO "usb: found %x:%x\n", ent->idVendor, ent->idProduct);

	/* Allocate a private structure and reference it as driver's data */
	data = kmalloc(sizeof(*data), GFP_KERNEL);
	if (data == NULL) {
		printk(KERN_WARNING "usb: unable to allocate private structure\n");
		ret = -ENOMEM;
		goto error;
	}
	memset(data, 0x00, sizeof(*data));

        usb_set_intfdata(interface, data);

	/* Init private field */
	kref_init(&data->kref);
	data->udev = usb_get_dev(interface_to_usbdev(interface));
	data->interface = interface;

	/* set up the endpoint information */
	/* use only the first interrupt endpoints */
	iface_desc = interface->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

                if (!data->int_in_endpointAddr &&
		    ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN) &&
		     ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT)) {
                        /* we found an interrupt in endpoint */
                        data->int_in_endpointAddr = endpoint->bEndpointAddress;
			data->int_in_size = le16_to_cpu(endpoint->wMaxPacketSize);
			data->int_in_interval = endpoint->bInterval;
		}

        }

        if (!(data->int_in_endpointAddr)) {
                printk(KERN_WARNING "usb: could not find an interrupt endpoint\n");
		ret = -ENODEV;
		goto error;
        }

        data->urb = usb_alloc_urb(0, GFP_KERNEL);
        if (!data->urb) {
		ret = -ENOMEM;
		goto error;
	}

	data->buffer = usb_buffer_alloc(data->udev, data->int_in_size,
					GFP_KERNEL, &data->urb->transfer_dma);
        if (!data->buffer) {
                ret = -ENOMEM;
		goto error;
        }

        usb_fill_int_urb(data->urb, data->udev,
			 usb_rcvintpipe(data->udev, data->int_in_endpointAddr),
			 data->buffer, data->int_in_size, k_complete, data,
			 data->int_in_interval);


	/* register the device now, as it is ready */
	ret = usb_register_dev(interface, &k_class);
        if (ret) {
                printk(KERN_WARNING "usb: not able to get a minor for this device.\n");
                goto error;
        }

	printk(KERN_INFO "usb: using minor %d for this device\n", interface->minor);

	return 0;

error:
	usb_set_intfdata(interface, NULL);
        if (data)
                kref_put(&data->kref, k_delete);
	return ret;
}

static void __devexit k_disconnect(struct usb_interface *interface)
{
	struct k_struct *data;

	data = usb_get_intfdata(interface);
	usb_deregister_dev(interface, &k_class);

	kref_put(&data->kref, k_delete);

	printk(KERN_INFO "usb: device removed\n");
}

static struct usb_driver k_driver = {
	.name =		"k_usb",
	.id_table =	k_id_table,
	.probe =	k_probe,
	.disconnect =	k_disconnect,
};

/*
 * Init and Exit
 */
static int __init k_init(void)
{
	int ret;

	ret = usb_register(&k_driver);
	if (ret < 0) {
		printk(KERN_WARNING "usb: unable to register usb driver\n");
		return ret;
	}

	return 0;
}

static void __exit k_exit(void)
{
	usb_deregister(&k_driver);
}

/*
 * Module entry points
 */
module_init(k_init);
module_exit(k_exit);
