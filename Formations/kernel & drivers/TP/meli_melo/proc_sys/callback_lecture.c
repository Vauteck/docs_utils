/************************************************************************\

  Callback de lecture depuis /proc

\************************************************************************/

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>

static char * nom_entree = "callback";

static int lecture (char *, char **, off_t, int, int *, void *);

static int __init chargement (void)
{
	struct proc_dir_entry * entree;
	
	entree = create_proc_entry(nom_entree, S_IFREG | 0644, & proc_root);
	if (entree == NULL)
		return -EBUSY;
	entree->owner  = THIS_MODULE;
	entree->read_proc = lecture;
	return 0; 
}

static void __exit dechargement (void)
{
	remove_proc_entry(nom_entree, & proc_root);
}

static int lecture (char *page, char **debut, off_t offset,
                    int maxi, int * eof, void * private)
{
	snprintf(page, maxi, "PID=%u, PPID=%u, UID=%u, Nom=%s\n",
	         current->pid, current->parent->pid, current->uid, current->comm);
	return strlen(page);
}

module_init(chargement);
module_exit(dechargement);

MODULE_LICENSE("GPL");
