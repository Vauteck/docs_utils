/************************************************************************\
  module_message_sol

  Exemple de module simple affichant un message sur la console du
  systeme lors de son chargement et de sa suppression.

\************************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

static int __init chargement (void)
{
	printk(KERN_INFO "Bonjour, je viens d'etre charge dans le noyau\n");
	return 0; 
}
 
 
static void __exit liberation (void)
{
	printk(KERN_INFO "Je suis supprime du noyau, au revoir !\n");
}

module_init(chargement);
module_exit(liberation);

MODULE_AUTHOR("TG");
MODULE_DESCRIPTION("Le plus simple des modules du noyau !");
MODULE_LICENSE("GPL");


