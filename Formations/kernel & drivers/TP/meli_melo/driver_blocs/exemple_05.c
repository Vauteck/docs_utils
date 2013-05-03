/************************************************************************\

  Secteurs de taille configurable au chargement

\************************************************************************/

#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/blkdev.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

	static char * nom_exemple = "exemple_05";

	static int majeur_exemple = 0;
	module_param_named(majeur, majeur_exemple, int, 0444);

	#define NB_MINEURS 16

	/* La taille des secteurs devient un parametre configurable,
	   avec la valeur par defaut 512. */

	#define LG_SECTEUR_DEFAUT 512
	static int lg_sect_exemple = LG_SECTEUR_DEFAUT;
	module_param_named(lg_sect, lg_sect_exemple, int, 0444);

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
		
		/*
		 * Les numeros de secteurs pour le transfert correspondent 
		 * a des secteurs de 512 octets... -> convertir.
		 */
		secteur_debut = rq->sector * 512 / lg_sect_exemple;
		nb_secteurs   = rq->current_nr_sectors * 512 / lg_sect_exemple;
		if (secteur_debut + nb_secteurs > nb_sect_exemple) {
			end_request(rq,1);
			continue;
		}
		if (rq_data_dir(rq)) /* write */
			memmove(& data_exemple[secteur_debut * lg_sect_exemple],
			        rq->buffer,
			        nb_secteurs * lg_sect_exemple);
		else /* read */
			memmove(rq->buffer,
			        & data_exemple[secteur_debut * lg_sect_exemple],
			        nb_secteurs * lg_sect_exemple);
		end_request(rq, 1);
	}
}

/*
 * Encore une conversion puisque l'on veut des secteurs de 512 octets
 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)

static int getgeo_exemple(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->heads = 4;
	geo->sectors = 8;
	geo->cylinders = nb_sect_exemple * lg_sect_exemple / 512 / geo->heads / geo->sectors;
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
	geometry.cylinders = nb_sect_exemple * lg_sect_exemple / 512 / geometry.heads / geometry.sectors;
	geometry.start = 0;
	if (copy_to_user((void *)arg, & geometry, sizeof(struct hd_geometry)))
		return -EFAULT;
	return 0;
}
#endif

static int __init chargement (void)
{
	int ret;

	if ((nb_sect_exemple <= 0) || (lg_sect_exemple <= 0))
		return -EINVAL;

	data_exemple = vmalloc(nb_sect_exemple * lg_sect_exemple);
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

	/* La capacite doit toujours etre indiquee en nombre de secteurs
	 * de 512 octets. Il faut donc faire une conversion. */

	set_capacity(gendisk_exemple, nb_sect_exemple * lg_sect_exemple / 512);
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

