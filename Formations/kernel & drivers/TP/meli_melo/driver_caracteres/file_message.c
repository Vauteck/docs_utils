/************************************************************************\
  
 Exemple de peripherique caractere

\************************************************************************/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/semaphore.h>

	static char * nom_module = "file_msg";

	static dev_t dev_file_msg = MKDEV(0, 0);

	static int numero = 0;
	module_param(numero, int, 0644);

	static struct cdev cdev_file_msg;

	static int read_msg  (struct file * filp, char * buffer,
                          size_t length, loff_t * offset);
	static int write_msg (struct file * filp, const char * buffer,
                          size_t length, loff_t * offset);

	static struct file_operations fops_file_msg = 
    {
		.owner   =  THIS_MODULE,
		.read    =  read_msg,
		.write   =  write_msg,
	};


	#define LG_MESSAGE_MAX 64

	typedef struct s_message {
	
	  char contenu [LG_MESSAGE_MAX];
	
	  struct s_message * suivant;
	
	} message_t;

	struct kmem_cache * cache_message = NULL;
	
	message_t * premier_message = NULL;

	DECLARE_MUTEX(mtx_premier_message);


static int __init chargement (void)
{
	int erreur;
	if (numero == 0) {
		erreur = alloc_chrdev_region(& dev_file_msg, 0, 1, nom_module);
	} else {
		dev_file_msg = MKDEV(numero, 0);
		erreur = register_chrdev_region(dev_file_msg, 1, nom_module);
	}
	if (erreur < 0)
		return erreur;

	cdev_init(& cdev_file_msg, & fops_file_msg);
	
	erreur = cdev_add(& cdev_file_msg, dev_file_msg, 1);
	if (erreur != 0)
		goto out_unregister;
	
	erreur = -ENOMEM;
	cache_message = kmem_cache_create(nom_module, sizeof(message_t), 0, 0, NULL, NULL);
	if (cache_message == NULL) 
		goto out_cdev_del;

	return 0;
	
out_cdev_del:
	cdev_del(& cdev_file_msg);

out_unregister:
	unregister_chrdev_region(dev_file_msg, 1);
	
	return erreur; 
}

static void __exit dechargement (void)
{
	message_t * suivant;

	while (premier_message != NULL) {
		suivant = premier_message->suivant;
		kmem_cache_free(cache_message, premier_message);
		premier_message = suivant;
	}
	kmem_cache_destroy(cache_message);
	cache_message = NULL;
	
	cdev_del(& cdev_file_msg);
	unregister_chrdev_region(dev_file_msg, 1);
}

static int read_msg(struct file * filp, char * buffer,
                    size_t length, loff_t * offset)
{
	int retour;
	int longueur;
	message_t * suivant;
	
	if (down_interruptible(& mtx_premier_message) != 0)
		return -ERESTARTSYS;
	
	retour = 0; /* 0 (=EOF), ou -EAGAIN */
	if (premier_message == NULL)
		goto out_up_mtx;
	
	longueur = strlen(premier_message->contenu) + 1;
	
	retour = -EINVAL;
	if (length < longueur)
		goto out_up_mtx;

	retour = -EFAULT;	
	if (copy_to_user(buffer, premier_message->contenu, longueur) != 0)
		goto out_up_mtx;
		
	retour = longueur;
	
	suivant = premier_message->suivant;
	
	kmem_cache_free(cache_message, premier_message);

	premier_message = suivant;
	
out_up_mtx:
	up(& mtx_premier_message);
	
	return retour;
}


static int write_msg(struct file * filp, const char * buffer,
                     size_t length, loff_t * offset)
{
	int retour;
	message_t * nouveau;
	message_t * precedent;
	
	if (down_interruptible(& mtx_premier_message) != 0)
		return -ERESTARTSYS;

	retour = -EINVAL;
	if (length >= LG_MESSAGE_MAX)
		goto out_up_mtx;

	retour = -ENOMEM;
	nouveau = kmem_cache_alloc(cache_message, 0);
	if (nouveau == NULL)
		goto out_up_mtx;

	retour = -EFAULT;	
	if (copy_from_user(nouveau->contenu, buffer, length) != 0) {
		kmem_cache_free(cache_message, nouveau);
		goto out_up_mtx;
	}
	
	nouveau->contenu[length] = '\0';

	nouveau->suivant = NULL;
	if (premier_message == NULL)
		premier_message = nouveau;
	else {
		for (precedent = premier_message; precedent->suivant != NULL; precedent = precedent->suivant)
			;
		precedent->suivant = nouveau;
	}
		
	retour = length;
	
out_up_mtx:
	up(& mtx_premier_message);
	
	return retour;
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");
