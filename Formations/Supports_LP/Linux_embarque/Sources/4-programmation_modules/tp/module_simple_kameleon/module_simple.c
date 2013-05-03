/***********************************
 Nom ....... : module_simple.c
 Objet ..... : Affichage de messages 
 Auteur .... : LP
 Date ...... : 04/12/2010
***********************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_AUTHOR("Moi");
MODULE_DESCRIPTION("Exemple de module simple");
MODULE_SUPPORTED_DEVICE("none");
MODULE_LICENSE("GPL");

/* Initialisation */
static int __init chargement_module(void)
{
    printk(KERN_DEBUG "Chargement de mon module...\n");
    return 0;
}

/* Liberation */
static void __exit dechargement_module(void)
{
    printk(KERN_DEBUG "Dechargement de mon module...\n");
}

module_init(chargement_module);
module_exit(dechargement_module);
