/************************************************************************\

  Fonction d'ioctl

\************************************************************************/

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include <asm/uaccess.h>

#include "exemple_05.h"
	
	static char * nom_module = "exemple_05";

	static dev_t dev_exemple = MKDEV(0, 0);

	static int numero = 0;
	module_param(numero, int, 0644);

	static struct cdev cdev_exemple;

	static int read_exemple  (struct file * filp, char * buffer,
	                          size_t length, loff_t * offset);
	static int ioctl_exemple (struct inode * ind, struct file * filp,
	                          unsigned int cmd, unsigned long arg);

	static int exemple_affiche_ppid = 1;

	static struct file_operations fops_exemple = {
		.owner   =  THIS_MODULE,
		.read    =  read_exemple,
		.ioctl   =  ioctl_exemple,
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

static int read_exemple(struct file * filp, char * buffer,
                        size_t length, loff_t * offset)
{
	char chaine[128];
	int l;

	if (exemple_affiche_ppid) 
		snprintf(chaine, 128, "PID = %u, PPID = %u\n",
		                current->pid, current->parent->pid);
	else
		snprintf(chaine, 128, "PID = %u\n", current->pid);

	l = strlen(chaine) + 1;
	l -= *offset;
	if (l <= 0)
		return 0;
	if (length < l)
		l = length;

	if (copy_to_user(buffer, & chaine[* offset], l) != 0)
		return -EFAULT;

	*offset += l;
	return l;
}

static int ioctl_exemple (struct inode * ind, struct file * filp,
	                      unsigned int cmd, unsigned long arg)
{
	if (_IOC_TYPE(cmd) != TYPE_IOCTL_EXEMPLE)
		return -ENOTTY;
		
	if (_IOC_DIR(cmd) & _IOC_READ)
		if (access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd)) == 0)
			return -EFAULT;
			
	if (_IOC_DIR(cmd) & _IOC_WRITE)
		if (access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd)) == 0)
			return -EFAULT;
			
	switch(_IOC_NR(cmd)) {
		case EX_GET_PPID :
			put_user(exemple_affiche_ppid, (int __user *) arg);
			break;
		case EX_SET_PPID :
			get_user(exemple_affiche_ppid, (int __user *) arg);
			break;
		default :
			return -ENOTTY; 
	}
	return 0;
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");

