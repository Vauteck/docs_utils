/* **************** LDD:1.0 s_09/lab5_hrtimer.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein and the Linux Foundation, 2009
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@linuxfoundation.org
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
 * High Resolution Timers
 *
 * Do the same things as in the first exercise, setting up two
 * periodic timers, but this time use the hrtimer interface.
 @*/

#include <linux/module.h>
#include <linux/timer.h>
#include <asm/msr.h>            /* needed for Time Stamp Counter functions */
#include <linux/init.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>

static unsigned long speed;     /* MHZ of CPU */

static struct kt_data
{
    struct hrtimer timer;
    ktime_t period;
    unsigned long start_time;   /* jiffies value when we first started the timer */
    u64 tsc_start;              /* TSC when timer was queued */
    u64 tsc_end;                /* TSC when timer is executed */
} *data_a, *data_b;

static void resubmit_it (struct hrtimer *var, struct kt_data *data)
{
    ktime_t now = var->base->get_time ();
    data->start_time = jiffies;
    hrtimer_forward (var, now, data->period);
}

static enum hrtimer_restart ktfun_a (struct hrtimer *var)
{
    u64 ticks;
    int msecs;
    rdtscll (data_a->tsc_end);
    ticks = data_a->tsc_end - data_a->tsc_start;
    msecs = (unsigned long)ticks / (speed * 1000);
    /* read the TSC for start time for the next cycle and resubmit */
    rdtscll (data_a->tsc_start);
    printk (KERN_INFO
            "A: period = %lld  elapsed = %ld TSC ticks: %lld msecs = %d\n",
            ktime_to_ns (data_a->period), jiffies - data_a->start_time, ticks,
            msecs);
    resubmit_it (var, data_a);
    return HRTIMER_NORESTART;
}

static enum hrtimer_restart ktfun_b (struct hrtimer *var)
{
    printk (KERN_INFO "   B: period = %lld  elapsed = %ld \n",
            ktime_to_ns (data_b->period), jiffies - data_b->start_time);
    /* resubmit */
    resubmit_it (var, data_b);
    return HRTIMER_NORESTART;
}

static int __init my_init (void)
{

    data_a = kmalloc (sizeof (*data_a), GFP_KERNEL);
    data_b = kmalloc (sizeof (*data_b), GFP_KERNEL);

    speed = cpu_khz / 1000;
    printk (KERN_INFO "CPU MHZ is found to be: %ld \n", speed);

    data_a->period = ktime_set (1, 0);  /* short period, 1 second  */
    data_b->period = ktime_set (10, 0); /* longer period, 10 seconds */

    hrtimer_init (&data_a->timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    hrtimer_init (&data_b->timer, CLOCK_REALTIME, HRTIMER_MODE_REL);

    data_a->timer.function = ktfun_a;
    data_b->timer.function = ktfun_b;

    /* initial timing info */
    data_a->start_time = jiffies;
    data_b->start_time = jiffies;
    rdtscll (data_a->tsc_start);

    hrtimer_start (&data_a->timer, data_a->period, HRTIMER_MODE_REL);
    hrtimer_start (&data_b->timer, data_b->period, HRTIMER_MODE_REL);

    return 0;
}
static void __exit my_exit (void)
{
    /* delete any running timers */
    printk (KERN_INFO "Deleted timer A: rc = %d\n",
            hrtimer_cancel (&data_a->timer));
    printk (KERN_INFO "Deleted timer B: rc = %d\n",
            hrtimer_cancel (&data_b->timer));
    kfree (data_a);
    kfree (data_b);
    printk (KERN_INFO "Module successfully unloaded \n");
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LDD:1.0 s_09/lab5_hrtimer.c");
MODULE_LICENSE ("GPL v2");
