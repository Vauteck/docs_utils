/************************************************************************\
  Chapitre "Peripheriques et protocoles reseau"
  exemple_02

  Affectation d'adresse materielle

  Exemples de la formation "Programmation Noyau sous Linux"
  (c) 2005-2007 Christophe Blaess - http://www.blaess.fr/christophe/

\************************************************************************/

#include <linux/module.h>
#include <linux/version.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>

	struct net_device * net_dev_ex = NULL;

static int open_exemple (struct net_device * net_dev)
{
	printk(KERN_INFO "%s: open_exemple(%p)\n",
	       THIS_MODULE->name, net_dev);
	/*
	   Remplir l'adresse physique en interrogeant le materiel.
	   Ici on la simule avec des valeurs arbitraires.
	   00:20:07:20:07:00
	*/
	net_dev->dev_addr[0] = 0x00;
	net_dev->dev_addr[1] = 0x20;
	net_dev->dev_addr[2] = 0x07;
	net_dev->dev_addr[3] = 0x20;
	net_dev->dev_addr[4] = 0x07;
	net_dev->dev_addr[5] = 0x00;

	/* Indiquer que l'interface peut traiter des paquets */
	netif_start_queue(net_dev);
	
	return 0;
}


static int stop_exemple (struct net_device * net_dev)
{
	printk(KERN_INFO "%s: stop_exemple(%p)\n",
	       THIS_MODULE->name, net_dev);

	/* L'interface ne peut plus traiter de paquets */
	netif_stop_queue(net_dev);
	
	return 0;
}


static void setup_exemple (struct net_device * net_dev)
{
	printk(KERN_INFO "%s: setup_exemple(%p)\n",
	       THIS_MODULE->name, net_dev);

	/* Objet de base : ethernet */
	ether_setup(net_dev);

	/* Surcharge de certaines methodes */
	net_dev->open = open_exemple;
	net_dev->stop = stop_exemple;
}


static void __exit dechargement(void);

static int __init chargement(void)
{
	printk(KERN_INFO "%s: chargement\n", THIS_MODULE->name);

	net_dev_ex = alloc_netdev(0, "ex%d", setup_exemple);
	if (net_dev_ex == NULL)
		return -ENOMEM;

	if (register_netdev(net_dev_ex) != 0) {
		dechargement();
		return -ENODEV;
	}
	return 0;
}


static void __exit dechargement(void)
{
	printk(KERN_INFO "%s: dechargement\n", THIS_MODULE->name);

	if (net_dev_ex != NULL) {
		unregister_netdev(net_dev_ex);
		free_netdev(net_dev_ex);
		net_dev_ex = NULL;
	}
}

module_init(chargement)
module_exit(dechargement)
MODULE_LICENSE("GPL");

