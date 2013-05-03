/************************************************************************\

  Driver permettant les entrees-sorties sur port parallele

\************************************************************************/

#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>

	static char * nom_module = "exemple_08";

	static dev_t dev_exemple = MKDEV(0, 0);

	static int numero = 0;
	module_param(numero, int, 0644);

	static struct cdev cdev_exemple;

	static int read_exemple  (struct file * filp, char * buffer,
	                          size_t length, loff_t * offset);
	static int write_exemple (struct file * filp, const char * buffer,
	                          size_t length, loff_t * offset);

	static struct file_operations fops_exemple = {
		.owner   =  THIS_MODULE,
		.read    =  read_exemple,
		.write   =  write_exemple,
	};

static int __init chargement (void)
{
	int erreur;

	if (request_region(888, 3, nom_module) == NULL)
		return -EBUSY;

	if (numero == 0) {
		erreur = alloc_chrdev_region(& dev_exemple, 0, 1, nom_module);
	} else {
		dev_exemple = MKDEV(numero, 0);
		erreur = register_chrdev_region(dev_exemple, 1, nom_module);
	}
	if (erreur < 0) {
		release_region(888, 3);
		return erreur;
	}

	cdev_init(& cdev_exemple, & fops_exemple);
	erreur = cdev_add(& cdev_exemple, dev_exemple, 1);
	if (erreur != 0) {
		unregister_chrdev_region(dev_exemple, 1);
		release_region(888, 3);
		return erreur;
	}
	return 0; 
}

static void __exit dechargement (void)
{
	cdev_del(& cdev_exemple);
	unregister_chrdev_region(dev_exemple, 1);
	release_region(888, 3);
}

static int read_exemple(struct file * filp, char * buffer,
                        size_t length, loff_t * offset)
{
	char chaine [4];
	if (length < 3)
		return 0;
	sprintf(chaine, "%02X ", inb(889));
	if (copy_to_user(buffer, chaine, 3) != 0)
		return -EFAULT;
	return 3;
}

static int write_exemple(struct file * filp, const char * buffer,
                         size_t length, loff_t * offset)
{
	char chaine[80];
	int v;

	if (length > 79)
		return -EINVAL;
	if (copy_from_user(chaine, buffer, length) != 0)
		return -EFAULT;
	if (sscanf(chaine, "%x", & v) != 1)
		return -EINVAL;
	outb(v, 888);
	return length;
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");
