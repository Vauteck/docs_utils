/************************************************************************\
  Chapitre "Peripheriques USB"
  exemple_07
  
  Driver pour carte d'entrees-sorties Velleman K8055

  Utilisation des zones de donnees privees

  Exemples de la formation "Programmation Noyau sous Linux"
  (c) 2005-2007 Christophe Blaess - http://www.blaess.fr/christophe/

\************************************************************************/

	#include <linux/version.h>
	#include <linux/module.h>
	#include <linux/usb.h>
	#include <asm/uaccess.h>

	#define ID_VENDEUR_EXEMPLE   0x10CF  /* Velleman  */
	#define ID_PRODUIT_EXEMPLE_1 0x5500  /* Kit K8055 */
	#define ID_PRODUIT_EXEMPLE_2 0x5501  /* Kit K8055 */
	#define ID_PRODUIT_EXEMPLE_3 0x5502  /* Kit K8055 */
	#define ID_PRODUIT_EXEMPLE_4 0x5503  /* Kit K8055 */

	static struct usb_device_id   id_table_exemple [] = {
		{ USB_DEVICE(ID_VENDEUR_EXEMPLE, ID_PRODUIT_EXEMPLE_1) },
		{ USB_DEVICE(ID_VENDEUR_EXEMPLE, ID_PRODUIT_EXEMPLE_2) },
		{ USB_DEVICE(ID_VENDEUR_EXEMPLE, ID_PRODUIT_EXEMPLE_3) },
		{ USB_DEVICE(ID_VENDEUR_EXEMPLE, ID_PRODUIT_EXEMPLE_4) },
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


	/*
	 * Les variables globables disparaissent, on les remplace
	 * par les champs d'une structure allouee dynamiquement.
	 */
	typedef struct {
		struct usb_device              * usb_device;
		int                              open_count;
		struct semaphore                 mutex;
		struct usb_endpoint_descriptor * intr_out_end;
		struct usb_endpoint_descriptor * intr_in_end;
		char                           * intr_out_buf;
		char                           * intr_in_buf;
		struct urb                     * intr_out_urb;
		struct urb                     * intr_in_urb;
		#define     INTERRUPT_OUT_SIZE_EXEMPLE 8
		#define     INTERRUPT_IN_SIZE_EXEMPLE  8
		unsigned char                    in_data_read[INTERRUPT_IN_SIZE_EXEMPLE];
		wait_queue_head_t                intr_out_queue;
		wait_queue_head_t                intr_in_queue;
		int                              intr_in_running;
		int                              intr_out_busy;
		int                              intr_in_done;
		spinlock_t                       read_lock;
	} exemple_t;

static int probe_exemple(struct usb_interface * intf,
                   const struct usb_device_id * dev_id)
{
	int i;
	int err;
	exemple_t * exemple;
	struct usb_host_interface * host_intf;
	struct usb_endpoint_descriptor * endpoint_desc;

	/* Allouer une structure exemple et initialiser ses champs. */

	exemple = kmalloc(sizeof(exemple_t), GFP_KERNEL);
	memset(exemple, 0, sizeof(exemple_t));
	sema_init(& (exemple->mutex), 1);
	spin_lock_init(& (exemple->read_lock));
	init_waitqueue_head(& (exemple->intr_in_queue));
	init_waitqueue_head(& (exemple->intr_out_queue));
	
	exemple->usb_device = usb_get_dev(interface_to_usbdev(intf));

	host_intf = intf->cur_altsetting;
		
	for (i = 0; i < host_intf->desc.bNumEndpoints; i++) {
		endpoint_desc = & (host_intf->endpoint[i].desc);
		
		if (endpoint_desc->bEndpointAddress & USB_DIR_IN) {
			switch (endpoint_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) {
				case USB_ENDPOINT_XFER_INT:
					exemple->intr_in_end = endpoint_desc;
					break;
				default:
					break;
			}
		} else {
			switch (endpoint_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) {
				case USB_ENDPOINT_XFER_INT:
					exemple->intr_out_end = endpoint_desc;
					break;
				default:
					break;
			}
		}
	}
	
	err = -ENODEV;
	if (exemple->intr_out_end == NULL) {
		printk(KERN_INFO "%s: no endpoint interrupt out in probe()\n",
		       THIS_MODULE->name);
		goto exit_null;
	}

	if (exemple->intr_in_end == NULL) {
		printk(KERN_INFO "%s: no endpoint interrupt in in probe()\n",
		       THIS_MODULE->name);
		goto exit_null;
	}

	err = -EINVAL;
	if (exemple->intr_out_end->wMaxPacketSize < INTERRUPT_OUT_SIZE_EXEMPLE) {
		printk(KERN_INFO "%s: Interrupt out max packet size too small\n",
		       THIS_MODULE->name);
		goto exit_null;
	}

	if (exemple->intr_in_end->wMaxPacketSize < INTERRUPT_IN_SIZE_EXEMPLE) {
		printk(KERN_INFO "%s: Interrupt in max packet size too small\n",
		       THIS_MODULE->name);
		goto exit_null;
	}

	err = -ENOMEM;
	exemple->intr_out_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (exemple->intr_out_urb == NULL)
		goto exit_null;

	exemple->intr_in_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (exemple->intr_in_urb == NULL)
		goto exit_free_out_urb;

	exemple->intr_out_buf = kmalloc(INTERRUPT_OUT_SIZE_EXEMPLE, GFP_KERNEL);
	if (exemple->intr_out_buf == NULL)
		goto exit_free_in_urb;
	
	exemple->intr_in_buf = kmalloc(INTERRUPT_IN_SIZE_EXEMPLE, GFP_KERNEL);
	if (exemple->intr_in_buf == NULL)
		goto exit_free_out_buf;

	err = usb_register_dev(intf, & class_driver_exemple);
	if (err != 0)
		goto exit_free_in_buf;

	printk(KERN_INFO "%s: Numero mineur : %d\n",
	       THIS_MODULE->name, intf->minor);

	/*
	 * Enregistrer la structure exemple dans un champ prive de la
	 * structure usb_interface.
	 */
	usb_set_intfdata(intf, exemple);
	
	return 0;

	exit_free_in_buf:
		kfree(exemple->intr_in_buf);
		exemple->intr_in_buf  = NULL;
	exit_free_out_buf:
		kfree(exemple->intr_out_buf);
		exemple->intr_out_buf = NULL;
	exit_free_in_urb:
		usb_free_urb(exemple->intr_in_urb);
		exemple->intr_in_urb  = NULL;
	exit_free_out_urb:
		usb_free_urb(exemple->intr_out_urb);
		exemple->intr_out_urb = NULL;
	exit_null:
		exemple->intr_in_end  = NULL;
		exemple->intr_out_end = NULL;
		exemple->usb_device   = NULL;
		kfree(exemple);
		return err;
}
	

static void disconnect_exemple(struct usb_interface * intf)
{
	exemple_t * exemple;
	
	usb_deregister_dev(intf, & class_driver_exemple);

	exemple = usb_get_intfdata(intf);
	usb_set_intfdata(intf, NULL);
	if (exemple == NULL)
		return;

	if (exemple->intr_in_running)
		usb_kill_urb(exemple->intr_in_urb);
	exemple->intr_in_running = 0;

	if (exemple->intr_out_busy)
		wait_event_interruptible_timeout(exemple->intr_out_queue, ! exemple->intr_out_busy, HZ);
	if (exemple->intr_out_busy)
		usb_kill_urb(exemple->intr_out_urb);	
	exemple->intr_out_busy = 0;	

	kfree(exemple->intr_in_buf);
	kfree(exemple->intr_out_buf);
	usb_free_urb(exemple->intr_in_urb);
	usb_free_urb(exemple->intr_out_urb);
	kfree(exemple);
}

static int open_exemple(struct inode *inode, struct file *filp)
{
	int err;
	exemple_t * exemple;
	struct usb_interface * intf;
	
	/* Rechercher l'interface USB associee au numero mineur. */
	intf = usb_find_interface(& usb_driver_exemple, iminor(inode));
	if (intf == NULL)
		return -ENODEV;
	
	/* Acceder a notre structure dynamique.*/
	exemple = usb_get_intfdata(intf);
	if (exemple == NULL)
		return -ENODEV;

	/*
	 * Sauver le pointeur sur notre structure dans un champ
	 * reserve de la structure file pour y acceder dans les
	 * appels-systeme read() et write().
	 */
	filp->private_data = exemple;

	if (down_interruptible(& exemple->mutex))
		return -ERESTARTSYS;
	if (exemple->open_count > 0) {
		up(& exemple->mutex);
		return -EBUSY;
	}
	if (exemple->usb_device == NULL) {
		up(& exemple->mutex);
		return -ENODEV;
	}
	spin_lock_init(& exemple->read_lock);
	exemple->intr_in_done = 0;

	/*
	 * Inscrire le pointeur dans le champ context de l'URB
	 * en INTERRUPT IN.
	 */
	usb_fill_int_urb(exemple->intr_in_urb,
	                 exemple->usb_device,
	                 usb_rcvintpipe(exemple->usb_device, exemple->intr_in_end->bEndpointAddress),
	                 exemple->intr_in_buf,
					 exemple->intr_in_end->wMaxPacketSize,
					 read_callback_exemple,
					 exemple,  /* <=== ici */
					 exemple->intr_in_end->bInterval);

	err = usb_submit_urb(exemple->intr_in_urb, GFP_KERNEL);
	exemple->intr_in_running = (err == 0);

	if (err == 0)	
		exemple->open_count ++;

	up(& exemple->mutex);
	return 0;
}


static int release_exemple(struct inode *inode, struct file *filp)
{
	exemple_t * exemple = (exemple_t *) (filp->private_data);

	if (exemple == NULL)
		return -ENODEV;
		
	if (down_interruptible(& exemple->mutex))
		return -ERESTARTSYS;
	if (exemple->intr_out_busy)
		wait_event_interruptible_timeout(exemple->intr_out_queue, ! exemple->intr_out_busy, HZ);
	if (exemple->intr_out_busy)
		usb_kill_urb(exemple->intr_out_urb);	
	exemple->intr_out_busy = 0;	
	if (exemple->intr_in_running)
		usb_kill_urb(exemple->intr_in_urb);
	exemple->intr_in_running = 0;
	exemple->open_count --;
	up(& exemple->mutex);
	return 0;
}


static ssize_t write_exemple(struct file * filp, const char __user * data, size_t length, loff_t * offset)
{
	int o0, o1, o2, o3, o4, o5, o6, o7;
	exemple_t * exemple = (exemple_t *) (filp->private_data);
	char * buffer;
	int err;

	if (exemple == NULL)
		return -ENODEV;

	buffer = kmalloc(length, GFP_KERNEL);
	if (buffer == NULL)
		return -ENOMEM;

	if (copy_from_user(buffer, data, length)) {
		kfree(buffer);
		return -EINVAL;
	}
	err = (sscanf(buffer, "%d %d %d %d %d %d %d %d", &o0, &o1, &o2, &o3, &o4, &o5, &o6, &o7) != 8);
	kfree(buffer);
	if (err)
		return -EINVAL;

	if (down_interruptible(& exemple->mutex))
		return -ERESTARTSYS;

	while(exemple->intr_out_busy)
		if (wait_event_interruptible_timeout(exemple->intr_out_queue, ! exemple->intr_out_busy, HZ) <= 0) {
			up(& exemple->mutex);
			return -ERESTARTSYS;
		}
	exemple->intr_out_busy = 1;


	exemple->intr_out_buf[0] = o0 & 0xFF; 
	exemple->intr_out_buf[1] = o1 & 0xFF;
	exemple->intr_out_buf[2] = o2 & 0xFF;
	exemple->intr_out_buf[3] = o3 & 0xFF;
	exemple->intr_out_buf[4] = o4 & 0xFF;
	exemple->intr_out_buf[5] = o5 & 0xFF;
	exemple->intr_out_buf[6] = o6 & 0xFF;
	exemple->intr_out_buf[7] = o7 & 0xFF;

	/*
	 * Inscrire egalement le pointeur dans le champ
	 * context de l'URB en INTERRUPT OUT.
	 */
	usb_fill_int_urb(exemple->intr_out_urb,
	                 exemple->usb_device,
	                 usb_sndintpipe(exemple->usb_device,
					        exemple->intr_out_end->bEndpointAddress),
	                 exemple->intr_out_buf,
					 INTERRUPT_OUT_SIZE_EXEMPLE,
					 write_callback_exemple,
					 exemple,
					 exemple->intr_out_end->bInterval);

	err = usb_submit_urb(exemple->intr_out_urb, GFP_KERNEL);
	up(& exemple->mutex);
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
	/* Recuperer le pointeur inscrit au remplissage de l'URB.  */
	exemple_t * exemple = (exemple_t *) urb->context;
	if (exemple == NULL)
		return;
	exemple->intr_out_busy = 0;
	wake_up_interruptible(& exemple->intr_out_queue);
}


static ssize_t read_exemple(struct file * filp, char __user * data,
                            size_t length, loff_t * offset)
{
	char chaine[64];
	exemple_t * exemple = (exemple_t *) filp->private_data;
	unsigned long irqsave;

	if (exemple == NULL)
		return -ENODEV;

	if ((exemple->usb_device == NULL)
	 || (exemple->intr_in_running == 0))
		return -ENODEV;
	while(! exemple->intr_in_done)
		if (wait_event_interruptible_timeout(exemple->intr_in_queue, exemple->intr_in_done, HZ)<= 0)
			return -ERESTARTSYS;
			
	spin_lock_irqsave(& exemple->read_lock, irqsave);
	exemple->intr_in_done = 0;
	snprintf(chaine, 64, "%d %d %d %d %d %d %d %d\n", 
	                0xFF & exemple->in_data_read[0],
					0xFF & exemple->in_data_read[1],
					0xFF & exemple->in_data_read[2],
					0xFF & exemple->in_data_read[3],
	                0xFF & exemple->in_data_read[4],
					0xFF & exemple->in_data_read[5],
					0xFF & exemple->in_data_read[6],
					0xFF & exemple->in_data_read[7]);
	spin_unlock_irqrestore(& exemple->read_lock, irqsave);

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
	exemple_t * exemple = (exemple_t *) (urb->context);
	if (exemple == NULL)
		return;

	spin_lock(& exemple->read_lock);
	memcpy(exemple->in_data_read, exemple->intr_in_buf, INTERRUPT_IN_SIZE_EXEMPLE);
	exemple->intr_in_done = 1;
	spin_unlock(& exemple->read_lock);
	
	if (exemple->usb_device != NULL)
		if (usb_submit_urb(urb, GFP_ATOMIC) != 0)
			exemple->intr_in_running = 0;

	wake_up_interruptible(& exemple->intr_in_queue);
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
