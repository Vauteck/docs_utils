/************************************************************************\
  module_parametres_sol

  Module acceptant des parametres lors de son chargement. Il affiche les
  valeurs des parametres sur la console au moment de son initialisation
  et de son extraction.

\************************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>


	static int entier = 1234;
	module_param(entier, int, 0644);
	MODULE_PARM_DESC(entier, "Parametre entier pour un module experimental");

	static char * ma_chaine = "Chaine_par_defaut";
	module_param_named(chaine, ma_chaine, charp, 0644);
	MODULE_PARM_DESC(chaine, "Parametre chaine de caracteres");
		
		
	static int ma_table[4] = { 0, 0, 0, 0 };
	static int nb_remplis = 0;
	module_param_array_named(table, ma_table, int, &nb_remplis, 0644);
	MODULE_PARM_DESC(table, "Parametre tableau de 4 entiers maxi");


static int __init chargement (void)
{
	printk(KERN_INFO "Mon parametre \"entier\" est %d\n", entier);
	printk(KERN_INFO "Mon parametre \"chaine\" est %s\n", ma_chaine);
	printk(KERN_INFO "Table : %d %d %d %d\n", ma_table[0], ma_table[1], ma_table[2], ma_table[3]);
	return 0; 
}

static void __exit dechargement(void)
{
	printk(KERN_INFO "Mon parametre \"entier\" est %d\n", entier);
	printk(KERN_INFO "Mon parametre \"chaine\" est %s\n", ma_chaine);
	printk(KERN_INFO "Table : %d %d %d %d\n", ma_table[0], ma_table[1], ma_table[2], ma_table[3]);
}
 
module_init(chargement);
module_exit(dechargement);

MODULE_AUTHOR("TG");
MODULE_DESCRIPTION("Un module acceptant des parametres");
MODULE_LICENSE("GPL");

