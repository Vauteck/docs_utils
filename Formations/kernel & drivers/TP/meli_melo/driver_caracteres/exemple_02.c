/************************************************************************\

  Fonctions d'ouverture et fermeture du fichier

\************************************************************************/

#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>

	static char * nom_module = "exemple_02";

	static dev_t dev_exemple = MKDEV(0, 0);

	static int numero = 0;
	module_param(numero, int, 0644);

	static struct cdev cdev_exemple;

	static int open_exemple  (struct inode * ind, struct file * filp);
	static int close_exemple (struct inode * ind, struct file * filp);

	static struct file_operations fops_exemple = {
		.owner   =  THIS_MODULE,
		.open    =  open_exemple,
		.release =  close_exemple,
	};

static int __init chargement (void)
{
	int erreur;
	if (numero == 0) {
		erreur = alloc_chrdev_region(& dev_exemple, 0, 1, nom_module);
	} else {
		dev_exemple = MKDEV(numero, 0);
		erreur = register_chrdev_region(dev_exemple, 1, nom_module);
	}
	if (erreur < 0)
		return erreur;

	cdev_init(& cdev_exemple, & fops_exemple);

	erreur = cdev_add(& cdev_exemple, dev_exemple, 1);
	if (erreur != 0) {
		unregister_chrdev_region(dev_exemple, 1);
		return erreur;
	}
	return 0; 
}

static void __exit dechargement (void)
{
	cdev_del(& cdev_exemple);
	unregister_chrdev_region(dev_exemple, 1);
}

static int open_exemple(struct inode * ind, struct file * filp)
{
	printk(KERN_INFO "open_exemple()\n");
	return 0;
}

static int close_exemple(struct inode * ind, struct file * filp)
{
	printk(KERN_INFO "close_exemple()\n");
	return 0;
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");

