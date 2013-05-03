/************************************************************************\

	driver_memoire_sol

	Exemple de pilote de peripherique qui enregistre la chaine envoyee
	lors d'une ecriture et la restitue en lecture. On utilise un buffer
	circulaire de taille limitee.

\************************************************************************/

	#include <linux/init.h>
	#include <linux/config.h>
	#include <linux/fs.h>
	#include <linux/kernel.h>
	#include <linux/module.h>
	#include <linux/version.h>

	#include <linux/cdev.h>
	#include <linux/vmalloc.h>
	#include <asm/uaccess.h>

	static ssize_t  read_periph   (struct file * filp, char * buffer, size_t length, loff_t * offset);
	static ssize_t  write_periph  (struct file * filp, const char * buffer, size_t length, loff_t * offset);


static struct file_operations fops = {
		.read    = read_periph,
		.write   = write_periph,
};

	#define LG_BUFFER_PERIPH       4096
	static char *  buffer_periph = NULL;
	static int     fin_buffer    = 0;

	static dev_t	dev;
	static struct cdev cdev;

	static int __init
chargement (void)
{
	int erreur;
	
	buffer_periph = vmalloc(LG_BUFFER_PERIPH);
	if (buffer_periph == NULL)
		return -ENOMEM;

	erreur = alloc_chrdev_region(& dev, 0, 1, "memoire");
	if (erreur < 0) {
		vfree(buffer_periph);
		return erreur;
	}

	cdev_init(& cdev, & fops);
		
	if ((erreur = cdev_add(& cdev, dev, 1)) != 0) {
		unregister_chrdev_region(dev, 1);
		vfree(buffer_periph);
		return erreur;
	}
	return 0;
}


	static void __exit
dechargement (void)
{
	cdev_del(& cdev);
	unregister_chrdev_region(dev, 1);
	vfree(buffer_periph);
}


	static ssize_t
read_periph (struct file * filp, char __user * buffer, size_t length, loff_t * offset)
{
	int n;
	if (fin_buffer == 0)
		return 0;
	if (fin_buffer <= length) {
		n = fin_buffer;
		copy_to_user(buffer, buffer_periph, fin_buffer);
		fin_buffer = 0;
	} else {
		n = length;
		copy_to_user(buffer, buffer_periph, length);
		memmove(buffer_periph, buffer_periph + length, fin_buffer - length);
		fin_buffer -= length;
	}
	return n;
}


	static ssize_t
write_periph (struct file * filp, const char __user * buffer, size_t length, loff_t * offset)
{
	int n;
	n = LG_BUFFER_PERIPH - fin_buffer;
	if (n == 0)
		return -EWOULDBLOCK;
	if (n > length) {
		copy_from_user(buffer_periph + fin_buffer, buffer, length);
		fin_buffer += length;
		return length;
	}
	copy_from_user(buffer_periph + fin_buffer, buffer, n);
	fin_buffer = LG_BUFFER_PERIPH;
	return n;
}


	module_init (chargement);
	module_exit (dechargement);

	MODULE_AUTHOR ("TG");
	MODULE_DESCRIPTION ("Pilote de peripherique minimal");
	MODULE_LICENSE ("GPL");

