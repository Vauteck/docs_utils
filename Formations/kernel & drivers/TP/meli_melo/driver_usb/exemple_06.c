/************************************************************************\
  Chapitre "Peripheriques USB"
  exemple_06
  
  Driver pour carte d'entrees-sorties Velleman K8055

  Fonctions de lecture des donnees

  Exemples de la formation "Programmation Noyau sous Linux"
  (c) 2005-2007 Christophe Blaess - http://www.blaess.fr/christophe/

\************************************************************************/

	#include <linux/version.h>
	#include <linux/module.h>
	#include <linux/usb.h>
	#include <asm/uaccess.h>

	#define ID_VENDEUR_EXEMPLE   0x10CF  /* Velleman  */
	#define ID_PRODUIT_EXEMPLE   0x5500  /* Kit K8055 */

	static struct usb_device_id   id_table_exemple [] = {
		{ USB_DEVICE(ID_VENDEUR_EXEMPLE, ID_PRODUIT_EXEMPLE) },
		{ }
	};
	MODULE_DEVICE_TABLE(usb, id_table_exemple);


	static int  probe_exemple      (struct usb_interface * intf,
	                                const struct usb_device_id * dev_id);
	static void disconnect_exemple (struct usb_interface * intf);
	
	static struct usb_driver usb_driver_exemple = {
		#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16)
		.owner      = THIS_MODULE,
		#endif
		.name       = "Velleman K8055",
		.id_table   = id_table_exemple,
		.probe      = probe_exemple,
		.disconnect = disconnect_exemple,
	};


	static int     open_exemple    (struct inode *, struct file *);
	static int     release_exemple (struct inode *, struct file *);
	static ssize_t write_exemple   (struct file *, const char __user *,
	                                size_t, loff_t *);
	static ssize_t read_exemple    (struct file *, char __user *,
	                                size_t, loff_t *);
	#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	static void write_callback_exemple (struct urb *, struct pt_regs *);
	static void read_callback_exemple  (struct urb *, struct pt_regs *);
	#else
	static void write_callback_exemple (struct urb *);
	static void read_callback_exemple  (struct urb *);
	#endif

	static struct file_operations file_operations_exemple = {
		.owner   = THIS_MODULE,
		.open    = open_exemple,
		.release = release_exemple,
		.write   = write_exemple,
		.read    = read_exemple,
	};

	static struct usb_class_driver class_driver_exemple = {
		.name = "usb/velleman%d",
		.fops = & file_operations_exemple,
		#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
		.mode = S_IFCHR | 0644,
		#endif
		.minor_base = 0,
	};

	static int open_count_exemple = 0;
	static DECLARE_MUTEX(mutex_exemple);

	static struct usb_device * usb_device_exemple = NULL;

	static struct usb_endpoint_descriptor * intr_out_end_exemple  = NULL;
	static struct urb                     * intr_out_urb_exemple  = NULL;
	static char                           * intr_out_buf_exemple  = NULL;
	static int                              intr_out_busy_exemple = 0;
	static          DECLARE_WAIT_QUEUE_HEAD(intr_out_queue_exemple);
	#define INTERRUPT_OUT_SIZE_EXEMPLE  8

	/*
	 * Le buffer intr_in_buffer sera rempli automatiquement par le
	 * sous-systeme usb-core puis la routine read_callback() sera invoquee.
	 * Elle recopiera le contenu du buffer dans in_data_read[] qui sera
	 * consulte lors des appels-systeme read().
	 */

	static struct usb_endpoint_descriptor * intr_in_end_exemple  = NULL;
	static struct urb                     * intr_in_urb_exemple  = NULL;
	static char                           * intr_in_buf_exemple  = NULL;
	static int                              intr_in_running_exemple = 0;
	static int                              intr_in_done_exemple      = 0;
	static spinlock_t                       read_lock_exemple;
	static          DECLARE_WAIT_QUEUE_HEAD(intr_in_queue_exemple);
	#define INTERRUPT_IN_SIZE_EXEMPLE  8
	static unsigned char                    in_data_read_exemple[INTERRUPT_IN_SIZE_EXEMPLE];


static int probe_exemple(struct usb_interface * intf,
                   const struct usb_device_id * dev_id)
{
	int i;
	int err;
	struct usb_host_interface * host_intf;
	struct usb_endpoint_descriptor * endpoint_desc;

	if (usb_device_exemple != NULL)
		printk(KERN_DEBUG "%s: usb_device not NULL in probe()\n",
		       THIS_MODULE->name);
	if (intr_out_end_exemple != NULL)
		printk(KERN_DEBUG "%s: intr_out_end not NULL in probe()\n",
		       THIS_MODULE->name);
	if (intr_in_end_exemple != NULL)
		printk(KERN_DEBUG "%s: intr_in_end not NULL in probe()\n",
		       THIS_MODULE->name);
	if (intr_out_urb_exemple != NULL)
		printk(KERN_DEBUG "%s: intr_out_urb not NULL in probe()\n",
		       THIS_MODULE->name);
	if (intr_in_urb_exemple != NULL)
		printk(KERN_DEBUG "%s: intr_in_urb not NULL in probe()\n",
		       THIS_MODULE->name);
	if (intr_out_buf_exemple != NULL)
		printk(KERN_DEBUG "%s: intr_out_buf not NULL in probe()\n",
		       THIS_MODULE->name);
	if (intr_in_buf_exemple != NULL)
		printk(KERN_DEBUG "%s: intr_in_buf not NULL in probe()\n",
		       THIS_MODULE->name);
		
	usb_device_exemple = usb_get_dev(interface_to_usbdev(intf));

	host_intf = intf->cur_altsetting;
	
	for (i = 0; i < host_intf->desc.bNumEndpoints; i++) {
		endpoint_desc = & (host_intf->endpoint[i].desc);

		if (endpoint_desc->bEndpointAddress & USB_DIR_IN) {
			switch (endpoint_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) {
				case USB_ENDPOINT_XFER_INT:
					intr_in_end_exemple = endpoint_desc;
					break;
				default:
					break;
			}
		} else {
			switch (endpoint_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) {
				case USB_ENDPOINT_XFER_INT:
					intr_out_end_exemple = endpoint_desc;
					break;
				default:
					break;
			}
		}
	}

	err = -ENODEV;
	if (intr_out_end_exemple == NULL) {
		printk(KERN_INFO "%s: no endpoint interrupt out in probe()\n",
		       THIS_MODULE->name);
		goto exit_null;
	}

	if (intr_in_end_exemple == NULL) {
		printk(KERN_INFO "%s: no endpoint interrupt in in probe()\n",
		       THIS_MODULE->name);
		goto exit_null;
	}

	err = -EINVAL;
	if (intr_out_end_exemple->wMaxPacketSize < INTERRUPT_OUT_SIZE_EXEMPLE) {
		printk(KERN_INFO "%s: Interrupt out max packet size too small\n",
		       THIS_MODULE->name);
		goto exit_null;
	}

	if (intr_in_end_exemple->wMaxPacketSize < INTERRUPT_IN_SIZE_EXEMPLE) {
		printk(KERN_INFO "%s: Interrupt in max packet size too small\n",
		       THIS_MODULE->name);
		goto exit_null;
	}

	err = -ENOMEM;
	intr_out_urb_exemple = usb_alloc_urb(0, GFP_KERNEL);
	if (intr_out_urb_exemple == NULL)
		goto exit_null;

	intr_in_urb_exemple = usb_alloc_urb(0, GFP_KERNEL);
	if (intr_in_urb_exemple == NULL)
		goto exit_free_out_urb;

	intr_out_buf_exemple = kmalloc(INTERRUPT_OUT_SIZE_EXEMPLE, GFP_KERNEL);
	if (intr_out_buf_exemple == NULL)
		goto exit_free_in_urb;
	
	intr_in_buf_exemple = kmalloc(INTERRUPT_OUT_SIZE_EXEMPLE, GFP_KERNEL);
	if (intr_in_buf_exemple == NULL)
		goto exit_free_out_buf;

	err = usb_register_dev(intf, & class_driver_exemple);
	if (err != 0)
		goto exit_free_in_buf;

	printk(KERN_INFO "%s: Numero mineur : %d\n",
	       THIS_MODULE->name, intf->minor);

	return 0;

	exit_free_in_buf:
		kfree(intr_in_buf_exemple);
		intr_in_buf_exemple  = NULL;
	exit_free_out_buf:
		kfree(intr_out_buf_exemple);
		intr_out_buf_exemple = NULL;
	exit_free_in_urb:
		usb_free_urb(intr_in_urb_exemple);
		intr_in_urb_exemple  = NULL;
	exit_free_out_urb:
		usb_free_urb(intr_out_urb_exemple);
		intr_out_urb_exemple = NULL;
	exit_null:
		intr_in_end_exemple  = NULL;
		intr_out_end_exemple = NULL;
		usb_device_exemple   = NULL;
		return err;
}


static void disconnect_exemple(struct usb_interface * intf)
{
	usb_deregister_dev(intf, & class_driver_exemple);
	/* Arreter les lecture INTERRUPT IN en cours de fonctionnement */
	if (intr_in_running_exemple)
		usb_kill_urb(intr_in_urb_exemple);
	intr_in_running_exemple = 0;

	if (intr_out_busy_exemple)
		wait_event_interruptible_timeout(intr_out_queue_exemple, ! intr_out_busy_exemple, HZ);
	if (intr_out_busy_exemple)
		usb_kill_urb(intr_out_urb_exemple);	
	intr_out_busy_exemple = 0;	

	kfree(intr_in_buf_exemple);
	kfree(intr_out_buf_exemple);
	usb_free_urb(intr_in_urb_exemple);
	usb_free_urb(intr_out_urb_exemple);
	intr_in_urb_exemple  = NULL;
	intr_out_urb_exemple = NULL;
	intr_in_buf_exemple  = NULL;
	intr_out_buf_exemple = NULL;
	intr_in_end_exemple  = NULL;
	intr_out_end_exemple = NULL;
	usb_device_exemple   = NULL;
}


static int open_exemple(struct inode *inode, struct file *filp)
{
	int err;
	
	if (down_interruptible(& mutex_exemple))
		return -ERESTARTSYS;
	if (open_count_exemple > 0) {
		up(& mutex_exemple);
		return -EBUSY;
	}
	if (usb_device_exemple == NULL) {
		up(& mutex_exemple);
		return -ENODEV;
	}
	spin_lock_init(& read_lock_exemple);
	intr_in_done_exemple = 0;

	/*
	 * Initialiser un URB pour la lecture INTERRUPT IN.
	 */
	usb_fill_int_urb(intr_in_urb_exemple,
	                 usb_device_exemple,
	                 usb_rcvintpipe(usb_device_exemple, intr_in_end_exemple->bEndpointAddress),
	                 intr_in_buf_exemple,
					 intr_in_end_exemple->wMaxPacketSize,
					 read_callback_exemple,
					 NULL,
					 intr_in_end_exemple->bInterval);

	/*
	 * Transmettre le URB. La fonction read_callback()
	 * sera invoquee lorsque la lecture sera terminée
	 * et disposera des donnees recues dans le
	 * buffer intr_in_buf.
	 */
	err = usb_submit_urb(intr_in_urb_exemple, GFP_KERNEL);
	/*
	 * Si la fonction precedente a renvoye zero, la lecture en
	 * interruption est lancee.
	 */
	intr_in_running_exemple = (err == 0);

	if (err == 0)	
		open_count_exemple ++;

	up(& mutex_exemple);
	return 0;
}


static int release_exemple(struct inode *inode, struct file *filp)
{
	if (down_interruptible(& mutex_exemple))
		return -ERESTARTSYS;
	if (intr_out_busy_exemple)
		wait_event_interruptible_timeout(intr_out_queue_exemple, ! intr_out_busy_exemple, HZ);
	if (intr_out_busy_exemple)
		usb_kill_urb(intr_out_urb_exemple);	
	intr_out_busy_exemple = 0;	
	if (intr_in_running_exemple)
		usb_kill_urb(intr_in_urb_exemple);
	intr_in_running_exemple = 0;
	open_count_exemple --;
	up(& mutex_exemple);
	return 0;
}


static ssize_t write_exemple (struct file * file,
     const char __user * data, size_t length, loff_t * offset)
{
	int o0, o1, o2, o3, o4, o5, o6, o7;
	char * buffer;
	int err;
	
	buffer = kmalloc(length, GFP_KERNEL);
	if (buffer == NULL) {
		return -ENOMEM;
	}
	if (copy_from_user(buffer, data, length)) {
		kfree(buffer);
		return -EINVAL;
	}
	err = (sscanf(buffer, "%d %d %d %d %d %d %d %d", &o0, &o1, &o2, &o3, &o4, &o5, &o6, &o7) != 8);
	kfree(buffer);
	if (err)
		return -EINVAL;

	if (down_interruptible(& mutex_exemple))
		return -ERESTARTSYS;

	while(intr_out_busy_exemple)
		if (wait_event_interruptible_timeout(intr_out_queue_exemple, ! intr_out_busy_exemple, HZ) <= 0) {
			up(& mutex_exemple);
			return -ERESTARTSYS;
		}
	intr_out_busy_exemple = 1;
	
	intr_out_buf_exemple[0] = o0 & 0xFF; 
	intr_out_buf_exemple[1] = o1 & 0xFF;
	intr_out_buf_exemple[2] = o2 & 0xFF;
	intr_out_buf_exemple[3] = o3 & 0xFF;
	intr_out_buf_exemple[4] = o4 & 0xFF;
	intr_out_buf_exemple[5] = o5 & 0xFF;
	intr_out_buf_exemple[6] = o6 & 0xFF;
	intr_out_buf_exemple[7] = o7 & 0xFF;

	usb_fill_int_urb(intr_out_urb_exemple,
	                 usb_device_exemple,
	                 usb_sndintpipe(usb_device_exemple,
					        intr_out_end_exemple->bEndpointAddress),
	                 intr_out_buf_exemple,
					 INTERRUPT_OUT_SIZE_EXEMPLE,
					 write_callback_exemple,
					 NULL,
					 intr_out_end_exemple->bInterval);

	err = usb_submit_urb(intr_out_urb_exemple, GFP_KERNEL);
	up(& mutex_exemple);
	if (err == 0)
		return length;
	return err;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static void write_callback_exemple(struct urb * urb, struct pt_regs * unused)
#else
static void write_callback_exemple(struct urb * urb)
#endif
{
	intr_out_busy_exemple = 0;
	wake_up_interruptible(& intr_out_queue_exemple);
}


static ssize_t read_exemple(struct file * file, char __user * data,
                            size_t length, loff_t * offset)
{
	char chaine[64];
	unsigned long irqsave;

	if ((usb_device_exemple == NULL)
	 || (intr_in_running_exemple == 0))
		/* Peripherique deconnecte */
		return -ENODEV;

	while(! intr_in_done_exemple)
		if (wait_event_interruptible_timeout(intr_in_queue_exemple, intr_in_done_exemple, HZ) <= 0)
			return -ERESTARTSYS;

	spin_lock_irqsave(& read_lock_exemple, irqsave);
	intr_in_done_exemple = 0;
	snprintf(chaine, 64, "%d %d %d %d %d %d %d %d\n", 
	                0xFF & in_data_read_exemple[0],
					0xFF & in_data_read_exemple[1],
					0xFF & in_data_read_exemple[2],
					0xFF & in_data_read_exemple[3],
	                0xFF & in_data_read_exemple[4],
					0xFF & in_data_read_exemple[5],
					0xFF & in_data_read_exemple[6],
					0xFF & in_data_read_exemple[7]);
	spin_unlock_irqrestore(& read_lock_exemple, irqsave);
	
	if (1+strlen(chaine) > length)
		return -ENOMEM;
		
	if (copy_to_user(data, chaine, 1+strlen(chaine)))
		return -EINVAL;

	return strlen(chaine);
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static void read_callback_exemple(struct urb * urb, struct pt_regs * unused)
#else
static void read_callback_exemple(struct urb * urb)
#endif
{
	spin_lock(& read_lock_exemple);
	/*
	 * Recopier les donnees recues se trouvant dans intr_in_buf
	 * dans une zone de stockage in_data_read où elles seront lues par
	 * l'appel-systeme read().
	 */
	memcpy(in_data_read_exemple, intr_in_buf_exemple, INTERRUPT_IN_SIZE_EXEMPLE);
	intr_in_done_exemple = 1;
	spin_unlock(& read_lock_exemple);
	
	/* Soumettre a nouveau l'URB pour relancer la prochaine lecture */
	if (usb_device_exemple != NULL)
		if (usb_submit_urb(urb, GFP_ATOMIC) != 0)
			intr_in_running_exemple = 0;

	/* Reveiller l'eventuel processus en attente dans l'appel-systeme read() */
	wake_up_interruptible(& intr_in_queue_exemple);
}


static int __init chargement(void)
{
	int err;

	err = usb_register(& usb_driver_exemple);
	if (err) {
		printk(KERN_ERR "%s: Erreur dans usb_register(): %d\n",
		       THIS_MODULE->name, err);
		return err;
	}
	return 0;
}


static void __exit dechargement(void)
{
	usb_deregister(& usb_driver_exemple);
}


module_init (chargement);
module_exit (dechargement);

MODULE_LICENSE("GPL");
