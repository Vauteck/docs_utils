/************************************************************************\

  Utilisation d'une tasklet

\************************************************************************/

#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/io.h>

	static char * nom_module = "exemple_10";

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	static irqreturn_t irq_handler(int irq, void * ident, struct pt_regs * unused);
#else
	static irqreturn_t irq_handler(int irq, void * ident);
#endif

#ifndef IRQF_SHARED
#define IRQF_SHARED SA_SHIRQ
#endif

	static void fonction_tasklet (unsigned long inutile);
	static DECLARE_TASKLET(tasklet_exemple, fonction_tasklet, 0);

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
	tasklet_kill(& tasklet_exemple);
	release_region(0x378, 3);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t irq_handler(int irq, void * ident, struct pt_regs * unused)
#else
static irqreturn_t irq_handler(int irq, void * ident)
#endif
{
	if ((inb(0x379) & 0x40) == 0)
		return IRQ_NONE;

	tasklet_schedule(& tasklet_exemple);

	return IRQ_HANDLED;
}

static void fonction_tasklet(unsigned long inutile)
{
	static int valeur = 0;

	outb(valeur, 0x378);
	valeur = 0xFF - valeur;
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");

