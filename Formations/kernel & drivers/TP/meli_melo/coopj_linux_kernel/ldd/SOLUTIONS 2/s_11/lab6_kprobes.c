/* **************** LDD:1.0 s_11/lab6_kprobes.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2009
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
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
 * Using kprobes to get statistics.
 *
 * Using kprobes, find out how often kernel timers are deleted before
 * they are run.
 *
 * Examination of the kernel source discloses that the exported
 * function __mod_timer() is called every time either add_timer() or
 * mod_timer() is called.
 *
 * You can see how often timers are deleted by monitoring del_timer()
 * and del_timer_sync(); however, on single processor systems,
 * del_timer_sync() is not defined.
 *
 * Timers are frequent so you'll probably won't want to print out
 * every time they are scheduled or deleted, but say every 100 times
 * plus final statistics.
 *
 * Is it possible that timer deletion can be more frequent than timer
 * scheduling?
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>

static struct kprobe kp_mod_timer, kp_del_timer;
static int count_mod_timer = 0, count_del_timer = 0, count_del_timer_sync = 0;

static int do_count (int *count, char *name)
{
    (*count)++;
    if ((*count) % 100 == 1) {
        printk (KERN_INFO "count_%s=%8d    ", name, *count);
        printk (KERN_INFO "current->comm=%s, current->pid=%d\n", current->comm,
                current->pid);
    }
    return 0;
}
static int h_mod_timer (struct kprobe *p, struct pt_regs *regs)
{
    return do_count (&count_mod_timer, "mod_timer");
}
static int h_del_timer (struct kprobe *p, struct pt_regs *regs)
{
    return do_count (&count_del_timer, "del_timer");
}

#ifdef CONFIG_SMP
static struct kprobe kp_del_timer_sync;
static int h_del_timer_sync (struct kprobe *p, struct pt_regs *regs)
{
    return do_count (&count_del_timer_sync, "del_timer_sync");
}
#endif

static int __init setup_probe (struct kprobe *kp, const char *name,
                               int (*h_pre) (struct kprobe * kp,
                                             struct pt_regs * regs))
{
    kp->pre_handler = h_pre;
    kp->symbol_name = (char *)name;
    printk (KERN_INFO "handler for %s at loaded\n", name);

    if (register_kprobe (kp)) {
        printk (KERN_INFO "Failed to register kprobe, quitting\n");
        return -1;
    }
    return 0;
}

static int __init my_init (void)
{
    /* set the handler functions */

    if (setup_probe (&kp_mod_timer, "__mod_timer", h_mod_timer))
        return -1;
    if (setup_probe (&kp_del_timer, "del_timer", h_del_timer)) {
        unregister_kprobe (&kp_mod_timer);
        return -1;
    }
#ifdef CONFIG_SMP
    if (setup_probe (&kp_del_timer_sync, "del_timer_sync", h_del_timer_sync)) {
        unregister_kprobe (&kp_mod_timer);
        unregister_kprobe (&kp_del_timer);
        return -1;
    }
#endif

    printk (KERN_INFO "Hello: module loaded at 0x%p\n", my_init);

    return 0;
}

static void __exit my_exit (void)
{
    unregister_kprobe (&kp_mod_timer);
    unregister_kprobe (&kp_del_timer);
#ifdef CONFIG_SMP
    unregister_kprobe (&kp_del_timer_sync);
#endif
    printk (KERN_INFO "\n\n FINAL STATISTICS:\n\n");
    printk (KERN_INFO "count_mod_timer = %d\n", count_mod_timer);
    printk (KERN_INFO "count_del_timer = %d\n", count_del_timer);
    printk (KERN_INFO "count_del_timer_sync = %d\n", count_del_timer_sync);
    printk (KERN_INFO "Bye: module unloaded from 0x%p\n", my_exit);
}

module_init (my_init);
module_exit (my_exit);
MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_11/lab6_kprobes.c");
MODULE_LICENSE ("GPL v2");
