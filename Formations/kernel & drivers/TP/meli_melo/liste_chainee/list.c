#include <linux/kernel.h>       /* printk() */
#include <linux/module.h>       /* modules */
#include <linux/init.h>         /* module_{init,exit} */
#include <linux/slab.h>         /* kmalloc/kfree */
#include <linux/list.h>		/* list */

MODULE_DESCRIPTION("tp-liste");
MODULE_AUTHOR("TG");
MODULE_LICENSE("GPL");

struct foo {
	int a;
	char b[10];
	struct list_head next;
};

static LIST_HEAD(head);

int __init k_init(void) {
	struct foo *elem;
	struct foo *elem1, *elem2;
	struct list_head *i;

	elem1 = (struct foo *)kmalloc(sizeof(struct foo), GFP_KERNEL);
	elem1->a = 1;
	strcpy(elem1->b, "toto");
	INIT_LIST_HEAD(&elem1->next);

	elem2 = (struct foo *)kmalloc(sizeof(struct foo), GFP_KERNEL);
	elem2->a = 2;
	strcpy(elem2->b, "zozo");
	INIT_LIST_HEAD(&elem2->next);

	list_add(&elem1->next, &head);
	list_add(&elem2->next, &head);

	printk("Premier parcours\n");
	list_for_each(i, &head) {
		elem = list_entry(i, struct foo, next);
		printk("L'element est %d, %s\n", elem->a, elem->b);
	}

	printk("Second parcours\n");
	list_for_each_entry(elem, &head, next) {
		printk("L'element est %d, %s\n", elem->a, elem->b);
	}
	return 0;
}

void __exit k_exit(void) {
	struct list_head *i, *tmp;

	list_for_each_safe(i, tmp, &head) {
		struct foo *elem;

		elem = list_entry(i, struct foo, next);
		printk("Effacement de l'element %d, %s\n", elem->a, elem->b);
		list_del(i);
		kfree(elem);
	}
}

module_init(k_init);
module_exit(k_exit);
