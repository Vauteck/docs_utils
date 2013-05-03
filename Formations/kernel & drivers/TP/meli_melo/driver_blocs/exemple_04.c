/************************************************************************\

  Traitement de ioctl() et getgeo() pour permettre le partitionnement

\************************************************************************/

#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/blkdev.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

	static char * nom_exemple = "exemple_04";

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

	static struct request_queue * request_queue_exemple;
	static struct gendisk       * gendisk_exemple;
	static spinlock_t             spinlock_exemple;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	static int getgeo_exemple  (struct block_device *,
	                            struct hd_geometry *);
#else
	static int ioctl_exemple   (struct inode *, struct file *,
	                            unsigned int, unsigned long);
#endif

	static struct block_device_operations devops_exemple = {
		.owner   = THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
		.getgeo  = getgeo_exemple,
#else
		.ioctl   = ioctl_exemple,
#endif
	};

static void request_exemple(request_queue_t * rqueue)
{
	unsigned long secteur_debut;
	unsigned long nb_secteurs;
	struct request * rq;

	while ((rq = elv_next_request(rqueue)) != NULL) {
		if (! blk_fs_request(rq)) {
			end_request(rq, 0);
			continue;
		}
		
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

		end_request(rq, 1);
	}
}


/* Les utilitaires de partitionnement, comme fdisk, appellent
 * la fonction ioctl(HDIO_GETGEO) pour connaitre la geometrie
 * du disque. Depuis le noyau 2.6.16, le sous-systeme Block
 * Devices convertit l'ioctl() en un appel de fonction
 * getgeo() de la structure block_device_operations.
 */
  

/* On simule un disque avec 4 tetes, 8 secteurs par cylindres,
 * et un nombre de cylindres dependant de sa capacite totale.
 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)

static int getgeo_exemple(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->heads = 4;
	geo->sectors = 8;
	geo->cylinders = nb_sect_exemple / geo->heads / geo->sectors;
	geo->start = 0;
	return 0;
}

#else

static int ioctl_exemple(struct inode * inode, struct file * filp,
                         unsigned int cmd, unsigned long arg)
{
	struct hd_geometry geometry;
	
	if (cmd != HDIO_GETGEO)
		return -ENOTTY;
	geometry.heads = 4;
	geometry.sectors = 8;
	geometry.cylinders = nb_sect_exemple / geometry.heads / geometry.sectors;
	geometry.start = 0;
	if (copy_to_user((void *)arg, & geometry, sizeof(struct hd_geometry)))
		return -EFAULT;
	return 0;
}
#endif

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

	spin_lock_init(& spinlock_exemple);

	request_queue_exemple = blk_init_queue(request_exemple,
	                                       & spinlock_exemple);
	if (request_queue_exemple == NULL) {
		unregister_blkdev(majeur_exemple, nom_exemple);
		vfree(data_exemple);
		return -ENOMEM;
	}

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
	
	add_disk(gendisk_exemple);
		
	return 0; 
}


static void __exit dechargement (void)
{
	del_gendisk(gendisk_exemple);
	blk_cleanup_queue(request_queue_exemple);
	unregister_blkdev(majeur_exemple, nom_exemple);
	vfree(data_exemple);
}

module_init(chargement);
module_exit(dechargement);
MODULE_LICENSE("GPL");

