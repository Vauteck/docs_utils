/************************************************************************\

  Gestion des interruptions du port parallele

\************************************************************************/

#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/io.h>

	static char * nom_module = "exemple_09";

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	static irqreturn_t irq_handler(int irq, void * ident, struct pt_regs * unused);
#else
	static irqreturn_t irq_handler(int irq, void * ident);
#endif

#ifndef IRQF_SHARED
#define IRQF_SHARED SA_SHIRD
#endif


static int __init chargement (void)
{
	int erreur;
	
	if (request_region(0x378, 3, nom_module) == NULL)
		return -EBUSY;

	erreur = request_irq(7, irq_handler, IRQF_SHARED, nom_module, nom_module);
	if (erreur != 0) {
		release_region(0x378, 3);
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
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t irq_handler(int irq, void * ident, struct pt_regs * unused)
#else
static irqreturn_t irq_handler(int irq, void * ident)
#endif

{
	static int valeur = 0;

	/* Verifions si la broche 10 est bien levee, ce qui indique
	   une interruption probable. */
	if ((inb(0x379) & 0x40) == 0)
		return IRQ_NONE;

	outb(valeur, 0x378);
	valeur = 0xFF - valeur;

	return IRQ_HANDLED;
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");
