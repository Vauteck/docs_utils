/************************************************************************\

	module_procfs_sol

	Module creant une entree /proc/perso. A chaque lecture dans ce
	fichier un compteur est incremente et sa valeur affichee. Lors
	de l'écriture d'un nombre décimal dans le fichier, la valeur
	du compteur est écrasŽe.

\************************************************************************/

	#include <linux/init.h>
	#include <linux/config.h>
	#include <linux/kernel.h>
	#include <linux/vmalloc.h>
	#include <linux/module.h>
	#include <linux/version.h>

	#include <asm/uaccess.h>
	#include <linux/proc_fs.h>

	static struct proc_dir_entry * entree_proc = NULL;

	#define LG_LIGNE	80

	static int
perso_read_proc (char * page, char ** debut, off_t offset,
		int nombre, int * eof, void * data)
{
	int * compteur = (int *) data;
	if (offset > 0)
		return 0;
	(* compteur) ++;
	snprintf (page, nombre, "Ca fait %d fois que vous lisez /proc/perso\n", * compteur);
	return 1 + strlen (page);
}

	static int
perso_write_proc (struct file * file, const char __user * buffer, unsigned long nombre, void * data)
{
	char ligne[LG_LIGNE];
	int n;
	int * compteur = (int *) data;
	
	if (nombre >= LG_LIGNE)
		return -ENOMEM;
	copy_from_user(ligne, buffer, nombre);
	ligne[nombre] = '\0';
	if (sscanf (ligne, "%d", & n) == 1) {
		* compteur = n;
		printk(KERN_INFO "Valeur %d ecrite dans /proc/perso\n", n);
		return nombre;
	}
	return -EINVAL;
}

	static int __init
chargement (void)
{
	int * compteur;

	if (entree_proc != NULL)
		return -EBUSY;

	compteur = vmalloc(sizeof(int));
	if (compteur == NULL)
		return -ENOMEM;
	* compteur = 0;

	entree_proc = create_proc_entry("perso",  S_IFREG | S_IRUGO | S_IWUSR, & proc_root);
	if (entree_proc == NULL)
		return -1;

	entree_proc->owner = THIS_MODULE;
	entree_proc->read_proc = perso_read_proc;
	entree_proc->write_proc = perso_write_proc;
	entree_proc->data = compteur;

	printk(KERN_INFO "Chargement du module procfs\n");

	return 0; 
}
 
 
	static void __exit
dechargement (void)
{
	void * ptr;
	printk (KERN_INFO "Dechargement du module procfs\n");

	if (entree_proc == NULL)
		return;
	ptr = entree_proc->data;
	remove_proc_entry("perso",  & proc_root);
	vfree(ptr);
	entree_proc = NULL;
}

	module_init(chargement);
	module_exit(dechargement);

	MODULE_AUTHOR("TG");
	MODULE_DESCRIPTION("Fichier experimental dans /proc");
	MODULE_LICENSE("GPL");

