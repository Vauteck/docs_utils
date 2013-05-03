/************************************************************************\

  Acces incontrole a une variable globale depuis un appel-systeme.

\************************************************************************/

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include <asm/uaccess.h>

	static char * nom_module = "exemple_06";

	static dev_t dev_exemple = MKDEV(0, 0);

	static int numero = 0;
	module_param(numero, int, 0644);

	static struct cdev cdev_exemple;

	static int read_exemple  (struct file * filp, char * buffer,
	                          size_t length, loff_t * offset);

	static struct file_operations fops_exemple = {
		.owner   =  THIS_MODULE,
		.read    =  read_exemple,
	};

	static int pid_current = 0;

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

	cdev_init(& cdev_exemple, & fops_exemple);
	erreur = cdev_add(& cdev_exemple, dev_exemple, 1);
	if (erreur != 0) {
		unregister_chrdev_region(dev_exemple, 1);
		return erreur;
	}
	return 0; 
}

static void __exit dechargement (void)
{
	cdev_del(& cdev_exemple);
	unregister_chrdev_region(dev_exemple, 1);
}

static int read_exemple(struct file * filp, char * buffer,
                        size_t length, loff_t * offset)
{
	char chaine[2];
	unsigned long limite;

	pid_current = current->pid;

	limite = jiffies + 10;
	
	/* Boucle de 10 ms pour provoquer artificiellement
	  la collision entre des appels-systeme simultanes.*/
	  
	while (time_before(jiffies, limite))
		schedule();
	/* on peut remplacer schedule() par cpu_relax() sur
	   un systeme entierement preemptible */

	if (pid_current == current->pid)
		strcpy(chaine, ".");
	else
		strcpy(chaine, "#");
	
	if (length < 2)
		return -ENOMEM;
	if (copy_to_user(buffer, chaine, 2) != 0)
		return -EFAULT;
		
	return 1;
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");
