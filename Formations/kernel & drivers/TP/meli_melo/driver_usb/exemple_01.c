/************************************************************************\
  Chapitre "Peripheriques USB"
  exemple_01
  
  Driver pour carte d'entrees-sorties Velleman K8055

  Enregistrement des fonctions de detection et extraction du peripherique

  Exemples de la formation "Programmation Noyau sous Linux"
  (c) 2005-2007 Christophe Blaess - http://www.blaess.fr/christophe/

\************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/usb.h>

/* Identification du peripherique gere par notre driver */
	
#define ID_VENDEUR_EXEMPLE   0x10CF  /* Velleman  */
#define ID_PRODUIT_EXEMPLE   0x5500  /* Kit K8055 */

	static struct usb_device_id   id_table_exemple [] = {
		{ USB_DEVICE(ID_VENDEUR_EXEMPLE, ID_PRODUIT_EXEMPLE) },
		{ } /* Par convention on termine par une entree vide */
	};
	MODULE_DEVICE_TABLE(usb, id_table_exemple);

	/* Fonctions de detection et deconnexion du peripherique */
	static int  probe_exemple      (struct usb_interface * intf,
	                                const struct usb_device_id * dev_id);
	static void disconnect_exemple (struct usb_interface * intf);

	/* Representation du pilote de peripherique */
	static struct usb_driver usb_driver_exemple = {
		#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16)
		.owner      = THIS_MODULE,
		#endif
		.name       = "Velleman K8055",
		.id_table   = id_table_exemple,
		.probe      = probe_exemple,
		.disconnect = disconnect_exemple,
	};



static int probe_exemple(struct usb_interface * intf,
                  const struct usb_device_id  * dev_id)
{
	/*
	 * Cette routine est appelee automatiquement par le sous-systeme
	 * usb-core du noyau lorsque le peripherique est branche.
	 * Elle doit renvoyer zero si elle est capable de piloter ce
	 * peripherique.
	 */
	printk(KERN_INFO "%s: probe_exemple() appelee\n",
	       THIS_MODULE->name);
	return 0;
}


static void disconnect_exemple(struct usb_interface * intf)
{
	/* 
	 * Cette fonction est invoquee par le sous-systeme usb-core
	 * lorsque le peripherique est debranche.
	 */
	printk(KERN_INFO "%s: disconnect_exemple() appelee\n",
	       THIS_MODULE->name);
}


static int __init chargement(void)

{
	/*
	 * A l'initialisation du module, on enregistre le driver usb.
	 * Attention, les fonctions probe() et disconnect() peuvent
	 * etre invoquee avant meme que usb_register() se termine.
	 */
	int err;

	err = usb_register(& usb_driver_exemple);
	if (err) {
		printk(KERN_ERR "%s: Erreur dans usb_register(): %d\n",
		       THIS_MODULE->name, err);
		return err;
	} else {
		printk(KERN_INFO "%s: Enregistrement du driver : Ok\n",
		       THIS_MODULE->name);
	}

	return 0;
}


static void __exit dechargement(void)
{
	/*
	 * Retrait du driver, et invocation de la routine disconnect().
	 */
	usb_deregister(& usb_driver_exemple);
}


module_init (chargement);
module_exit (dechargement);

MODULE_LICENSE("GPL");
