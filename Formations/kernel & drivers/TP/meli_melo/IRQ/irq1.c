/************************************************************************\

	module_irq_sol

	Ce module installe un gestionnaire pour l'interruption declenchee par
	un front montant sur la broche 10 du port parallele.  Le gestionnaire
	bascule l'etat de la broche 2 de ce port.

\************************************************************************/

#include <linux/init.h>
#include <linux/config.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <asm/io.h>

static irqreturn_t handler_irq (int irq, void * inutile, struct pt_regs * regs)
{
	static int sortie = 0;
	outb(sortie, 0x378);
	sortie = 1 - sortie;
	return IRQ_HANDLED;
}

static int __init chargement(void)
{	
	int r;
	if ((r = request_irq(7, handler_irq, 0, "port_parallele", NULL)) != 0)
		return r;
	outb(0x10, 0x37A);
	
	return 0;
}

void __exit dechargement(void)
{
	outb(0x00, 0x37A);
	free_irq(7, NULL);
}

module_init(chargement);
module_exit(dechargement);

MODULE_AUTHOR("TG");
MODULE_DESCRIPTION("Gestionnaire experimental d'interruption");
MODULE_LICENSE("GPL");

