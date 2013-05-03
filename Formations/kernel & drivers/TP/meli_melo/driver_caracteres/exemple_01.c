/************************************************************************\

  Reservation des numeros mineur et majeur

\************************************************************************/

#include <linux/module.h>
#include <linux/fs.h>

	static char * nom_module = "exemple_01";

	static dev_t dev_exemple = MKDEV(0, 0);

	static int numero = 0;
	module_param(numero, int, 0644);

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
	return 0; 
}

static void __exit dechargement (void)
{
	unregister_chrdev_region(dev_exemple, 1);
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");

