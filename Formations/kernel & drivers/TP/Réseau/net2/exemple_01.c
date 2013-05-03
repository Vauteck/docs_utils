/************************************************************************\
  Chapitre "Peripheriques et protocoles reseau"
  exemple_01

  Enregistrement d'un peripherique reseau

  Exemples de la formation "Programmation Noyau sous Linux"
  (c) 2005-2007 Christophe Blaess - http://www.blaess.fr/christophe/

\************************************************************************/

#include <linux/module.h>
#include <linux/version.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>

	struct net_device * net_dev_ex = NULL;

static void __exit dechargement(void);

static int __init chargement(void)
{
	printk(KERN_INFO "%s: chargement\n", THIS_MODULE->name);

	net_dev_ex = alloc_netdev(0, "ex%d", ether_setup);
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

