
#include <linux/version.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

static char * nom_entree = "current";

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
	snprintf(page, maxi,
	         "uid=%d, euid=%d, suid=%d, fsuid=%d\n"
	         "gid=%d, egid=%d, sgid=%d, fsgid=%d\n",
		current->uid, current->euid, current->suid, current->fsuid,
		current->gid, current->egid, current->sgid, current->fsgid);
	
	return strlen(page);
}

module_init(chargement);
module_exit(dechargement);

MODULE_LICENSE("GPL");
