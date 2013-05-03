/************************************************************************\

  Creation d'une classe de peripherique

\************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/io.h>

	static char * nom_module = "exemple_18";

	static dev_t dev_exemple = MKDEV(0, 0);

	static int numero = 0;
	module_param(numero, int, 0644);

	static struct cdev cdev_exemple;

	static int mmap_exemple (struct file * filp, struct vm_area_struct * vm);

	static void fonction_timer (unsigned long arg);

	static struct file_operations fops_exemple = {
		.owner   =  THIS_MODULE,
		.mmap    =  mmap_exemple,
	};

	static char * buffer = NULL;

	struct timer_list timer_exemple;

	static struct class * class_exemple = NULL;

static int __init chargement (void)
{
	int erreur;
	struct page * pg;
	
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

	buffer = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (buffer == NULL) {
		cdev_del(& cdev_exemple);
		unregister_chrdev_region(dev_exemple, 1);
		return -EBUSY;
	}

	pg = virt_to_page(buffer);
	SetPageReserved(pg);

	buffer[0] = '\0';

	init_timer(& timer_exemple);
	timer_exemple.function = fonction_timer;
	timer_exemple.expires = jiffies + HZ;
	add_timer(& timer_exemple);

	class_exemple = class_create(THIS_MODULE, "exemple");
	if (IS_ERR(class_exemple)) {
		del_timer(& timer_exemple);
		kfree(buffer);
		buffer = NULL;
		cdev_del(& cdev_exemple);
		unregister_chrdev_region(dev_exemple, 1);
	}
	class_device_create(class_exemple, NULL, dev_exemple, NULL,
	                    "exemple_%d", 18);

	return 0; 
}

static void __exit dechargement (void)
{
	class_device_destroy(class_exemple, dev_exemple);
	class_destroy(class_exemple);
	del_timer(& timer_exemple);
	kfree(buffer);
	buffer = NULL;
	cdev_del(& cdev_exemple);
	unregister_chrdev_region(dev_exemple, 1);
}


static int mmap_exemple (struct file * filp, struct vm_area_struct * vma)
{
	int err;

	if ((unsigned long) (vma->vm_end - vma->vm_start) > PAGE_SIZE)
		return -EINVAL;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
	err = remap_pfn_range(vma,
	                    (unsigned long) (vma->vm_start),
	                    virt_to_phys(buffer) >> PAGE_SHIFT,
	                    PAGE_SIZE,
	                    PAGE_SHARED);
#else
	err = remap_page_range(vma,
	                    (unsigned long) (vma->vm_start),
	                    virt_to_phys(buffer),
	                    PAGE_SIZE,
	                    PAGE_SHARED);
#endif
	if (err != 0)
		return -EAGAIN;
	return 0;
}

static void fonction_timer (unsigned long arg)
{
	sprintf(buffer, "\r%ld", jiffies);
	mod_timer(& timer_exemple, jiffies + HZ);
}

	module_init(chargement);
	module_exit(dechargement);

	MODULE_LICENSE("GPL");

