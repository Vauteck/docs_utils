/************************************************************************\

  Creation et suppression d'une entree dans /proc

\************************************************************************/

#include <linux/module.h>
#include <linux/proc_fs.h>

static char * nom_entree = "exemple_09";

static int __init chargement (void)
{
	struct proc_dir_entry * entree;
	
	entree = create_proc_entry(nom_entree, S_IFREG | 0644, & proc_root);
	if (entree == NULL)
		return -EBUSY;
	entree->owner  = THIS_MODULE;
	return 0; 
}

static void __exit dechargement (void)
{
	remove_proc_entry(nom_entree, & proc_root);
}

module_init(chargement);
module_exit(dechargement);

MODULE_LICENSE("GPL");
