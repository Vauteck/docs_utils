/************************************************************************\

  Reservation de memoire

\************************************************************************/

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/vmalloc.h>

	static char * nom_exemple = "exemple_02";

	static int majeur_exemple = 0;
	module_param_named(majeur, majeur_exemple, int, 0444);

	/* Pour le noyau la taille des secteurs est toujours 512 octets.
	 * Pour l'instant nous conservons cette taille. */
	#define LG_SECTEUR 512

	/* Le nombre de secteurs est configurable au chargement du module.
	 * Par defaut, il vaut 4096 soit un disque de 2Mo. */

	static int nb_sect_exemple = 4096;
	module_param_named(nb_sect, nb_sect_exemple, int, 0444);

	static char * data_exemple = NULL;

static int __init chargement (void)
{
	int ret;

	if (nb_sect_exemple <= 0)
		return -EINVAL;

	data_exemple = vmalloc(nb_sect_exemple * LG_SECTEUR);
	if (data_exemple == NULL)
		return -ENOMEM;

	ret = register_blkdev(majeur_exemple, nom_exemple);
	if (ret < 0) {
		vfree(data_exemple);
		return ret;
	}

	if (majeur_exemple == 0)
		majeur_exemple = ret;

	return 0; 
}

static void __exit dechargement (void)
{
	unregister_blkdev(majeur_exemple, nom_exemple);
	vfree(data_exemple);
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");

