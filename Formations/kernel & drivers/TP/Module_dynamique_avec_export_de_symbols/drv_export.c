#include <linux/module.h>                                       

#include "export_api.h"                                    

static int __init export_chargement(void)                             
{
	printk(KERN_ALERT "Hello, world\n");                                                 
        return 0;                                                  
}
                                               
static void __exit export_dechargement(void)
{
      printk(KERN_ALERT "Goodbye, cruel world\n");                                            
}
                                                                
void fonction_hello(int numero)                                 
{
        printk (KERN_INFO "Hello, le numero est %d\n", numero); 
}                                                               

EXPORT_SYMBOL(fonction_hello);

MODULE_LICENSE("GPL");

module_init(export_chargement);
module_exit(export_dechargement);
