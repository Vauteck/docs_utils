/************************************************************************\

  Appels-systeme d'acces au disque

\************************************************************************/

#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

	static char * nom_exemple = "exemple_03";

	static int majeur_exemple = 0;
	module_param_named(majeur, majeur_exemple, int, 0444);

	/* Le numero mineur 0 correspond au disque dans son entier.
	 * Les numeros 1, 2, etc. correspondent aux partitions creees sur le disque.
	 * Le nombre maximal de mineurs influe sur le nombre maximal de partitions.
	 */
	#define NB_MINEURS 16

	#define LG_SECTEUR 512
	static int nb_sect_exemple = 4096;
	module_param_named(nb_sect, nb_sect_exemple, int, 0444);

	static char * data_exemple = NULL;

	/* La structure request_queue stocke les requetes en attente */
	static struct request_queue * request_queue_exemple;
	
	/* La structure gendisk represente un disque generique.*/
	static struct gendisk * gendisk_exemple;

	/* Le spinlock est utilise en interne par le sous-systeme block
	 * pour limiter les acces concurrents.*/	
	static spinlock_t spinlock_exemple;

	static int open_exemple    (struct inode *, struct file *);
	static int release_exemple (struct inode *, struct file *);

	static struct block_device_operations devops_exemple = {
		.owner   = THIS_MODULE,
		.open    = open_exemple,
		.release = release_exemple,
	};

static int open_exemple(struct inode * inode, struct file * filp)
{
	/* Cette fonction est appelee a chaque ouverture du peripherique.
	 * Elle n'a pas d'utilite ici. */
	printk(KERN_INFO "open_exemple()\n");
	return 0;
}

static int release_exemple(struct inode * inode, struct file * filp)
{
	printk(KERN_INFO "release_exemple()\n");
	return 0;
}

static void request_exemple(request_queue_t * rqueue)
{
	unsigned long secteur_debut;
	unsigned long nb_secteurs;
	struct request * rq;

	/* Lire les requetes de la file. */
	while ((rq = elv_next_request(rqueue)) != NULL) {
		/* Ignorer les requetes autres que celles du
		 * systeme de fichier. */
		if (! blk_fs_request(rq)) {
			end_request(rq, 0);
			continue;
		}
		
		/* Rechercher la position des donnees a transferer. */
		secteur_debut = rq->sector;
		nb_secteurs   = rq->current_nr_sectors;
		if (secteur_debut + nb_secteurs > nb_sect_exemple) {
			end_request(rq,1);
			continue;
		}

		if (rq_data_dir(rq)) /* write */
			memmove(& data_exemple[secteur_debut*LG_SECTEUR],
			        rq->buffer,
			        nb_secteurs*LG_SECTEUR);
		else /* read */
			memmove(rq->buffer,
			        & data_exemple[secteur_debut*LG_SECTEUR],
			        nb_secteurs*LG_SECTEUR);

		/* Valider le traitement de la requete. */
		end_request(rq, 1);
	}
}

static int __init chargement (void)
{
	int ret;

	if (nb_sect_exemple <= 0)
		return -EINVAL;

	data_exemple = vmalloc(nb_sect_exemple * LG_SECTEUR);
	if (data_exemple == NULL)
		return -ENOMEM;

	ret = register_blkdev(majeur_exemple, nom_exemple);
	if (ret < 0) {
		vfree(data_exemple);
		return ret;
	}

	if (majeur_exemple == 0)
		majeur_exemple = ret;

	/* Initialiser le spinlock et la file des requetes. */
	spin_lock_init(& spinlock_exemple);

	request_queue_exemple = blk_init_queue(request_exemple,
	                                       & spinlock_exemple);
	if (request_queue_exemple == NULL) {
		unregister_blkdev(majeur_exemple, nom_exemple);
		vfree(data_exemple);
		return -ENOMEM;
	}

	/* Initialiser le driver gendisk */
	gendisk_exemple = alloc_disk(NB_MINEURS);
	if (gendisk_exemple == NULL) {
		blk_cleanup_queue(request_queue_exemple);
		unregister_blkdev(majeur_exemple, nom_exemple);
		vfree(data_exemple);
		return -ENOMEM;
	}
	gendisk_exemple->major       = majeur_exemple;
	gendisk_exemple->first_minor = 0;
	gendisk_exemple->fops        = & devops_exemple;
	gendisk_exemple->queue       = request_queue_exemple;
	snprintf(gendisk_exemple->disk_name, 32, nom_exemple);
	set_capacity(gendisk_exemple, nb_sect_exemple);
	
	/* Enregistrer le driver gendisk */
	add_disk(gendisk_exemple);
		
	return 0; 
}

static void __exit dechargement (void)
{
	/* Supprimer le driver gendisk. */
	del_gendisk(gendisk_exemple);

	/* Vider la file des requetes.*/
	blk_cleanup_queue(request_queue_exemple);

	unregister_blkdev(majeur_exemple, nom_exemple);
	vfree(data_exemple);
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");

