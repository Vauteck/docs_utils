/* **************** CLDD:1.0 s_10/lab3_probe_module.c **************** */
/*
 * The code herein is: Copyright 1998-2009 by Jerry Cooperstein
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 * URL:    http://www.coopj.com
 * email:  coop@coopj.com 
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */

/*
 * Probing a module
 *
 * Take an earlier module (such as a character driver) and add both
 * kprobes and jprobes instrumentation to it.
 *
 * Does the function you are probing need to be exported to be
 * accessible to the probe utiltities?
 *
 @*/

#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>

static int mycdrv_open_probe (struct inode *inode, struct file *file)
{
    printk (KERN_INFO "\n\n ****JPROBE**** in mycdrv_open_debug\n\n");
    jprobe_return ();
    return 0;
}

static struct kprobe kp;

static int handler_pre (struct kprobe *p, struct pt_regs *regs)
{
    printk (KERN_INFO "pre_handler: p->addr=0x%p\n", p->addr);
    return 0;
}

static void handler_post (struct kprobe *p, struct pt_regs *regs,
                          unsigned long flags)
{
    printk (KERN_INFO "post_handler: p->addr=0x%p\n", p->addr);
}

static int handler_fault (struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    printk (KERN_INFO "fault_handler:p->addr=0x%p\n", p->addr);
    return 0;
}

static struct jprobe jp = {
    .kp.symbol_name = "mycdrv_open",
    .entry = (kprobe_opcode_t *) mycdrv_open_probe,
};

static int __init my_init (void)
{
    /* set the handler functions */

    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;
    kp.fault_handler = handler_fault;
    kp.symbol_name = "mycdrv_open";

    if (register_kprobe (&kp)) {
        printk (KERN_INFO "Failed to register kprobe, quitting\n");
        return -1;
    }
    register_jprobe (&jp);
    printk (KERN_INFO "plant jprobe at %p, handler addr %p\n", jp.kp.addr,
            jp.entry);
    return 0;
}
static void __exit my_exit (void)
{

    unregister_jprobe (&jp);
    unregister_kprobe (&kp);
    printk (KERN_INFO "k,jprobes unregistered\n");
}

module_init (my_init);
module_exit (my_exit);
MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("CLDD:1.0 s_10/lab3_probe_module.c");
MODULE_LICENSE ("GPL v2");
