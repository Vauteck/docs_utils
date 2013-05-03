/************************************************************************\

  Enregistrement du peripherique bloc

\************************************************************************/

#include <linux/module.h>
#include <linux/fs.h>

	static char * nom_exemple = "exemple_01";

	static int majeur_exemple = 0;
	module_param_named(majeur, majeur_exemple, int, 0444);

static int __init chargement (void)
{
	int ret;

	ret = register_blkdev(majeur_exemple, nom_exemple);
	if (ret < 0)
		return ret;

	if (majeur_exemple == 0)
		/* Memoriser le numero attribue par le noyau */
		majeur_exemple = ret;

	return 0; 
}

static void __exit dechargement (void)
{
	unregister_blkdev(majeur_exemple, nom_exemple);
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");


