/************************************************************************\

  Synchronisation par spinlock entre appel-systeme et interruption

\************************************************************************/

#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>

	static char * nom_module = "exemple_12";

	static dev_t dev_exemple = MKDEV(0, 0);

	static int numero = 0;
	module_param(numero, int, 0644);

	static struct cdev cdev_exemple;

	static int read_exemple  (struct file * filp, char * buffer,
	                          size_t length, loff_t * offset);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	static irqreturn_t irq_handler(int irq, void * ident, struct pt_regs * unused);
#else
	static irqreturn_t irq_handler(int irq, void * ident);
#endif

#ifndef IRQF_SHARED
#define IRQF_SHARED SA_SHIRQ
#endif

	static struct file_operations fops_exemple = {
		.owner   =  THIS_MODULE,
		.read    =  read_exemple,
	};


	#define LG_BUFFER_EXEMPLE 1024
	static unsigned char buffer_exemple[LG_BUFFER_EXEMPLE];
	static int           fin_buffer_exemple = 0;
	static spinlock_t    lock_buffer_exemple;

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

	spin_lock_init(& lock_buffer_exemple);

	cdev_init(& cdev_exemple, & fops_exemple);

	erreur = cdev_add(& cdev_exemple, dev_exemple, 1);
	if (erreur != 0) {
		unregister_chrdev_region(dev_exemple, 1);
		return erreur;
	}

	if (request_region(0x378, 3, nom_module) == NULL) {
		cdev_del(& cdev_exemple);
		unregister_chrdev_region(dev_exemple, 1);
		return -EBUSY;
	}

	erreur = request_irq(7, irq_handler, IRQF_SHARED, nom_module, nom_module);
	if (erreur != 0) {
		release_region(0x378, 3);
		cdev_del(& cdev_exemple);
		unregister_chrdev_region(dev_exemple, 1);
		return erreur;
	}
	outb(0x10, 0x37A);
	return 0; 

}

static void __exit dechargement (void)
{
	outb(0, 0x37A);
	free_irq(7, nom_module);
	release_region(0x378, 3);
	cdev_del(& cdev_exemple);
	unregister_chrdev_region(dev_exemple, 1);
}


static int read_exemple(struct file * filp, char * buffer,
                        size_t length, loff_t * offset)
{
	unsigned long sauvegarde_it;
	char chaine[4];

	if (length < 4)
		return 0;

	spin_lock_irqsave(& lock_buffer_exemple, sauvegarde_it);

	if (fin_buffer_exemple == 0) {
		spin_unlock_irqrestore(& lock_buffer_exemple, sauvegarde_it);
		return 0;
	}
	
	sprintf(chaine, "%02X ", (int) (buffer_exemple[0]));
	fin_buffer_exemple --;
	if (fin_buffer_exemple > 0)
		memmove(buffer_exemple, & (buffer_exemple[1]), fin_buffer_exemple);
	
	spin_unlock_irqrestore(& lock_buffer_exemple, sauvegarde_it);

	if (copy_to_user(buffer, chaine, 4) != 0)
		return -EFAULT;
	return 4;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	static irqreturn_t irq_handler(int irq, void * ident, struct pt_regs * unused)
#else
	static irqreturn_t irq_handler(int irq, void * ident)
#endif
{
	int valeur = 0;

	valeur = inb(0x379);
	if ((valeur & 0x40) == 0)
		return IRQ_NONE;
	valeur &= 0xBF; /* retirer le bit 0x40 */

	spin_lock(& lock_buffer_exemple);
	if (fin_buffer_exemple < LG_BUFFER_EXEMPLE) {
		buffer_exemple[fin_buffer_exemple] = valeur;
		fin_buffer_exemple ++;
	}
	spin_unlock(& lock_buffer_exemple);
	return IRQ_HANDLED;
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");
