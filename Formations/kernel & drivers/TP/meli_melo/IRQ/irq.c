/*
 * Includes
 */
#include <linux/kernel.h>	/* printk() */
#include <linux/module.h>	/* modules */
#include <linux/init.h>		/* module_{init,exit} */
#include <linux/interrupt.h>	/* interrupts */
#include <asm/io.h>		/* {in,out}b */
#include <linux/slab.h>		/* kmalloc/kfree */
#include <linux/version.h>

MODULE_DESCRIPTION("tp-irg");
MODULE_AUTHOR("tg");
MODULE_LICENSE("GPL");

/*
 * Constantes
 */
#define KBD_IRQ	1

/*
 * Global variables
 */
struct data {
	unsigned char scancode;
	struct list_head next;
};

static LIST_HEAD(fifo);
static spinlock_t lock = SPIN_LOCK_UNLOCKED;

/*
 * Event handlers
 */
static void k_bh_handler(unsigned long code)
{
	struct list_head *idx;
	struct list_head *temp;

	spin_lock_irq(&lock);
	list_for_each_safe(idx, temp, &fifo) {
		struct data *item;

		item = list_entry(idx, struct data, next);
		list_del(idx);
		spin_unlock_irq(&lock);

		printk("irq: scancode = %x -> %s.\n",
			item->scancode,
			item->scancode & 0x80 ? "RELEASED" : "PRESSED");

		kfree(item);

		spin_lock_irq(&lock);
	}
	spin_unlock_irq(&lock);
}

DECLARE_TASKLET(k_tasklet, k_bh_handler, 0);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
static irqreturn_t k_irq_handler(int irq, void *dev_id)
#else
static irqreturn_t k_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
	struct data *item;
	unsigned char scancode;

	/* Read keyboard status and then the scancode */
	inb(0x64);
	scancode = inb(0x60);

	item = kmalloc(sizeof(struct data), GFP_ATOMIC);
	if (item) {
		item->scancode = scancode;
		spin_lock(&lock);
		list_add_tail(&item->next, &fifo);
		spin_unlock(&lock);

		/* Schedule the tasklet */
		tasklet_schedule(&k_tasklet);
	}

	return IRQ_HANDLED;
}

/*
 * Init and Exit
 */
static int __init k_init(void)
{
	int ret;

	/* Uninstall the previous keyboard handler */
	//free_irq(KBD_IRQ, NULL);

	/* Install our own keyboard handler */
	ret = request_irq(KBD_IRQ, k_irq_handler, SA_SHIRQ, "new_keyboard", k_irq_handler);
	if (ret < 0) {
		printk(KERN_WARNING "irq: unable to register the kbd irq\n");

		return ret;
	}

	printk(KERN_INFO "irq successfully loaded\n");

	return 0;
}

static void __exit k_exit(void)
{
	free_irq(KBD_IRQ, k_irq_handler);

	printk(KERN_INFO "irq successfully unloaded\n");
}


/*
 * Module entry points
 */
module_init(k_init);
module_exit(k_exit);
