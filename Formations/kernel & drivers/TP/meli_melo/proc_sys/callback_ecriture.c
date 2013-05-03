/************************************************************************\

  Callback d'ecriture dans /proc

\************************************************************************/

#include <linux/sched.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#include <asm/uaccess.h>

static char * nom_entree = "exemple_11";

static int valeur_inscrite = 0;

static int lecture (char *, char **, off_t, int, int *, void *);
static int ecriture (struct file *, const char __user *, unsigned long, void *);

static int __init chargement (void)
{
	struct proc_dir_entry * entree;
	
	entree = create_proc_entry(nom_entree, S_IFREG | 0644, & proc_root);
	if (entree == NULL)
		return -EBUSY;
	entree->owner  = THIS_MODULE;
	entree->read_proc = lecture;
	entree->write_proc = ecriture;
	return 0; 
}

static void __exit dechargement (void)
{
	remove_proc_entry(nom_entree, & proc_root);
}

static int lecture (char *page, char **debut, off_t offset,
                    int maxi, int * eof, void * private)
{
	snprintf(page, maxi, "PID=%u, PPID=%u, valeur_inscrite=%d\n",
	         current->pid, current->parent->pid, valeur_inscrite);
	return strlen(page);
}

static int ecriture (struct file * filp, const char __user * u_page,
                     unsigned long nombre, void * data)
{
	char buffer[128];
	if (strlen_user(u_page) >= 128)
		return -ENOMEM;
	if (copy_from_user(buffer, u_page, strlen_user(u_page)+1) != 0)
		return -ENOMEM;
	if (sscanf(buffer, "%d", & valeur_inscrite) != 1)
		return -EINVAL;
	return strlen_user(u_page)+1;
}

module_init(chargement);
module_exit(dechargement);

MODULE_LICENSE("GPL");
