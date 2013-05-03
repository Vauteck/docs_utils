/************************************************************************\
  Chapitre "Peripheriques USB"
  exemple_03
  
  Driver pour carte d'entrees-sorties Velleman K8055

  Acces en ecriture depuis l'espace utilisateur.

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
	#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	static void write_callback_exemple (struct urb *, struct pt_regs *);
	#else
	static void write_callback_exemple (struct urb *);
	#endif

	static struct file_operations file_operations_exemple = {
		.owner   = THIS_MODULE,
		.open    = open_exemple,
		.release = release_exemple,
		.write   = write_exemple,
	};

	static struct usb_class_driver class_driver_exemple = {
		.name = "usb/velleman%d",
		.fops = & file_operations_exemple,
		#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
		.mode = S_IFCHR | 0644,
		#endif
		.minor_base = 0,
	};

	/* Donnees utilisees par le driver */	

	static struct usb_device              * usb_device_exemple   = NULL;
	static struct usb_endpoint_descriptor * intr_out_end_exemple = NULL;
	static struct urb                     * intr_out_urb_exemple = NULL;
	static char                           * intr_out_buf_exemple = NULL;
	#define INTERRUPT_OUT_SIZE_EXEMPLE  8


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
	if (intr_out_urb_exemple != NULL)
		printk(KERN_DEBUG "%s: intr_out_urb not NULL in probe()\n",
		       THIS_MODULE->name);
	if (intr_out_buf_exemple != NULL)
		printk(KERN_DEBUG "%s: intr_out_buf not NULL in probe()\n",
		       THIS_MODULE->name);
	
	/* Stocker l'identifiant du peripherique. */	

	usb_device_exemple = usb_get_dev(interface_to_usbdev(intf));

	host_intf = intf->cur_altsetting;
	
	/* Rechercher un endpoint INTERRUPT OUT */

	for (i = 0; i < host_intf->desc.bNumEndpoints; i++) {
		endpoint_desc = & (host_intf->endpoint[i].desc);
		if (! (endpoint_desc->bEndpointAddress & USB_DIR_IN)) {
			switch (endpoint_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) {
				case USB_ENDPOINT_XFER_INT:
					intr_out_end_exemple = endpoint_desc;
					break;
				default:
					break;
			}
		}
	}
	
	if (intr_out_end_exemple == NULL) {
		printk(KERN_INFO "%s: no endpoint interrupt out in probe()\n",
		       THIS_MODULE->name);
		return -ENODEV;
	}

	/* Verifier si le endpoint INTERRUPT OUT a une bande passante suffisante. */
	if (intr_out_end_exemple->wMaxPacketSize < INTERRUPT_OUT_SIZE_EXEMPLE) {
		printk(KERN_INFO "%s: Interrupt out max packet size too small\n",
		       THIS_MODULE->name);
		intr_out_end_exemple = NULL;
		usb_device_exemple = NULL;
		return -EINVAL;
	}

	/* Allouer un URB pour les requetes en ecriture a venir */
	intr_out_urb_exemple = usb_alloc_urb(0, GFP_KERNEL);
	if (intr_out_urb_exemple == NULL) {
		intr_out_end_exemple = NULL;
		usb_device_exemple = NULL;
		return -ENOMEM;
	}

	/*
	 * Allouer un buffer pour le transfert des donnees en sortie.
	 * Attention ce buffer doit etre alloue avec kmalloc() ou la
	 * fonction usb_buffer_alloc() mais pas en variable statique.
	 */
	intr_out_buf_exemple = kmalloc(INTERRUPT_OUT_SIZE_EXEMPLE, GFP_KERNEL);
	if (intr_out_buf_exemple == NULL) {
		usb_free_urb(intr_out_urb_exemple);
		intr_out_urb_exemple = NULL;
		intr_out_end_exemple = NULL;
		usb_device_exemple = NULL;
		return -ENOMEM;
	}
	
	/*
	 * L'enregistrement du peripherique permet d'y acceder par un
	 * fichier special caractere de numero majeur USB (generalement 180)
	 * et de numero mineur alloue par le noyau a partir de celui indique
	 * dans la structure class_driver.
	 */
	err = usb_register_dev(intf, & class_driver_exemple);
	if (err != 0) {
		kfree(intr_out_buf_exemple);
		intr_out_buf_exemple = NULL;
		usb_free_urb(intr_out_urb_exemple);
		intr_out_urb_exemple = NULL;
		intr_out_end_exemple = NULL;
		usb_device_exemple = NULL;
		return err;
	}

	/* A present nous connaissons le numero mineur attribue, on l'affiche. */
	printk(KERN_INFO "%s: Numero mineur : %d\n",
	       THIS_MODULE->name, intf->minor);
	return 0;
}


static void disconnect_exemple(struct usb_interface * intf)
{
	/* Realiser les operations inverses de probe(). */
	usb_deregister_dev(intf, & class_driver_exemple);
	kfree(intr_out_buf_exemple);
	usb_free_urb(intr_out_urb_exemple);
	intr_out_urb_exemple = NULL;
	intr_out_buf_exemple = NULL;
	intr_out_end_exemple = NULL;
	usb_device_exemple = NULL;
}


static int open_exemple(struct inode *inode, struct file *filp)
{
	return 0;
}


static int release_exemple(struct inode *inode, struct file *filp)
{
	return 0;
}


static ssize_t write_exemple (struct file * file,
     const char __user * data, size_t length, loff_t * offset)
{
	int o0, o1, o2, o3, o4, o5, o6, o7;
	char * buffer;
	int err;
	
	/* Allouer un buffer de travail contenant une copie des donnees
	 * envoyees par l'utilisateur. */
	buffer = kmalloc(length, GFP_KERNEL);
	if (buffer == NULL) {
		return -ENOMEM;
	}
	if (copy_from_user(buffer, data, length)) {
		kfree(buffer);
		return -EINVAL;
	}
	/* On attend 8 chiffres decimaux à envoyer à la carte. */
	err = (sscanf(buffer, "%d %d %d %d %d %d %d %d", &o0, &o1, &o2, &o3, &o4, &o5, &o6, &o7) != 8);
	/* Le buffer peut etre libere immediatement. */
	kfree(buffer);
	if (err)
		return -EINVAL;
	
	/* On ecrit les donnees a envoyer dans le buffer alloue
	   pour la sortie INTERRUPT OUT. */
	intr_out_buf_exemple[0] = o0 & 0xFF; 
	intr_out_buf_exemple[1] = o1 & 0xFF;
	intr_out_buf_exemple[2] = o2 & 0xFF;
	intr_out_buf_exemple[3] = o3 & 0xFF;
	intr_out_buf_exemple[4] = o4 & 0xFF;
	intr_out_buf_exemple[5] = o5 & 0xFF;
	intr_out_buf_exemple[6] = o6 & 0xFF;
	intr_out_buf_exemple[7] = o7 & 0xFF;

	/* Remplir l'URB de INTERRUPT OUT pour une ecriture. */
	usb_fill_int_urb(intr_out_urb_exemple,
	                 usb_device_exemple,
	                 usb_sndintpipe(usb_device_exemple,
					        intr_out_end_exemple->bEndpointAddress),
	                 intr_out_buf_exemple,
					 INTERRUPT_OUT_SIZE_EXEMPLE,
					 write_callback_exemple,
					 NULL, /* donnees privees non utilisees */
					 intr_out_end_exemple->bInterval);

	/* Soumettre la requete URB au sous-systeme usb-core. */
	err = usb_submit_urb(intr_out_urb_exemple, GFP_KERNEL);
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
	/*
	 * Cette routine est appelee une fois que la requete
	 * associee a l'URB est terminee. Dans cette premiere
	 * version on ne fait rien.
	 */
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
