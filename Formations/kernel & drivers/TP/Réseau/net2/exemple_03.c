/************************************************************************\
  Chapitre "Peripheriques et protocoles reseau"
  exemple_03

  Enregistrement d'un double peripherique

  Exemples de la formation "Programmation Noyau sous Linux"
  (c) 2005-2007 Christophe Blaess - http://www.blaess.fr/christophe/

  Librement inspire d'un exemple du livre "Linux Device Driver"
  d'Alessandro Rubini et Jonathan Corbet.

\************************************************************************/

#include <linux/module.h>
#include <linux/version.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>

	struct net_device * net_dev_ex_0 = NULL;
	struct net_device * net_dev_ex_1 = NULL;

static int open_exemple (struct net_device * net_dev)
{
	printk(KERN_INFO "%s: open_exemple(%p)\n",
	       THIS_MODULE->name, net_dev);

	net_dev->dev_addr[0] = 0x00;
	net_dev->dev_addr[1] = 0x20;
	net_dev->dev_addr[2] = 0x07;
	net_dev->dev_addr[3] = 0x20;
	net_dev->dev_addr[4] = 0x07;
	/* Les deux adresses physiques different sur le dernier octet */
	if (net_dev == net_dev_ex_0)
		net_dev->dev_addr[5] = 0x00;
	else
		net_dev->dev_addr[5] = 0x01;
		
	netif_start_queue(net_dev);
	return 0;
}


static int stop_exemple (struct net_device * net_dev)
{
	printk(KERN_INFO "%s: stop_exemple(%p)\n",
	       THIS_MODULE->name, net_dev);

	netif_stop_queue(net_dev);
	return 0;
}


static void setup_exemple (struct net_device * net_dev)
{
	printk(KERN_INFO "%s: setup_exemple(%p)\n",
	       THIS_MODULE->name, net_dev);

	ether_setup(net_dev);

	net_dev->open = open_exemple;
	net_dev->stop = stop_exemple;
}


static void __exit dechargement(void);

static int __init chargement(void)
{
	printk(KERN_INFO "%s: chargement\n", THIS_MODULE->name);

	net_dev_ex_0 = alloc_netdev(0, "ex%d", setup_exemple);
	if (net_dev_ex_0 == NULL)
		return -ENOMEM;

	if (register_netdev(net_dev_ex_0) != 0) {
		dechargement();
		return -ENODEV;
	}

	net_dev_ex_1 = alloc_netdev(0, "ex%d", setup_exemple);
	if (net_dev_ex_1 == NULL) {
		dechargement();
		return -ENOMEM;
	}

	if (register_netdev(net_dev_ex_1) != 0) {
		dechargement();
		return -ENODEV;
	}
	return 0;
}


static void __exit dechargement(void)
{
	printk(KERN_INFO "%s: dechargement\n", THIS_MODULE->name);

	if (net_dev_ex_1 != NULL) {
		unregister_netdev(net_dev_ex_1);		
		free_netdev(net_dev_ex_1);
		net_dev_ex_1 = NULL;
	}

	if (net_dev_ex_0 != NULL) {
		unregister_netdev(net_dev_ex_0);
		free_netdev(net_dev_ex_0);
		net_dev_ex_0 = NULL;
	}
}

module_init(chargement)
module_exit(dechargement)
MODULE_LICENSE("GPL");
