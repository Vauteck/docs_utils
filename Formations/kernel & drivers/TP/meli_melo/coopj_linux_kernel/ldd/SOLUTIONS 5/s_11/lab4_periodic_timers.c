/* **************** CLDD:1.0 s_11/lab4_periodic_timers.c **************** */
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
 * Multiple Periodic Kernel Timers
 *
 * Write a module that launches two periodic kernel timer functions;
 * i.e., they should re-install themselves.
 *
 * One periodic sequence should be for less than 256 ticks (so it
 * falls in the tv1 vector), and the other should be for less than 16
 * K ticks (so it falls in the tv2 vector.)
 *
 * Each time the timer functions execute, print out the total elapsed
 * time since the module was loaded (in jiffies).
 *
 * For one of the functions, also read the TSC and calibrate with the
 * CPU frequency (as read from /proc/cpuinfo or the cpu_khz> variable)
 * to print out the elapsed time (hopefully) more accurately.
 *
 @*/

#include <linux/module.h>
#include <linux/timer.h>
#include <asm/msr.h>            /* needed for Time Stamp Counter functions */
#include <linux/init.h>
#include <linux/jiffies.h>

static unsigned long speed;     /* MHZ of CPU */

static struct timer_list timer_a, timer_b;

static struct kt_data
{
    unsigned long period;
    unsigned long start_time;   /* jiffies value when we first started the timer */
    u64 tsc_start;              /* TSC when timer was queued */
    u64 tsc_end;                /* TSC when timer is executed */
} *data_a, *data_b;

static void ktfun_a (unsigned long var)
{
    u64 ticks;
    int msecs;
    struct kt_data *tdata = (struct kt_data *)var;

    rdtscll (tdata->tsc_end);
    ticks = tdata->tsc_end - tdata->tsc_start;
    msecs = (unsigned long)ticks / (speed * 1000);
    printk (KERN_INFO
            "A: period = %ld  elapsed = %ld TSC ticks: %lld msecs = %d\n",
            tdata->period, jiffies - tdata->start_time, ticks, msecs);

    /* read the TSC for start time for the next cycle and resubmit */
    rdtscll (tdata->tsc_start);
    mod_timer (&timer_a, tdata->period + jiffies);
}
static void ktfun_b (unsigned long var)
{
    struct kt_data *tdata = (struct kt_data *)var;

    printk (KERN_INFO "   B: period = %ld  elapsed = %ld \n", tdata->period,
            jiffies - tdata->start_time);

    /* resubmit */
    mod_timer (&timer_b, tdata->period + jiffies);
}
static int __init my_init (void)
{
    speed = cpu_khz / 1000;
    printk (KERN_INFO "CPU MHZ is found to be: %ld \n", speed);

    init_timer (&timer_a);
    init_timer (&timer_b);

    timer_a.function = ktfun_a;
    timer_b.function = ktfun_b;

    data_a = kmalloc (sizeof (*data_a), GFP_KERNEL);
    data_b = kmalloc (sizeof (*data_b), GFP_KERNEL);

    timer_a.data = (unsigned long)data_a;
    timer_b.data = (unsigned long)data_b;

    data_a->period = 1 * HZ;    /* short period, 1 second  */
    data_b->period = 10 * HZ;   /* longer period, 10 seconds */

    data_a->start_time = jiffies;
    data_b->start_time = jiffies;

    timer_a.expires = jiffies + data_a->period;
    timer_b.expires = jiffies + data_b->period;

    rdtscll (data_a->tsc_start);

    add_timer (&timer_a);
    add_timer (&timer_b);

    return 0;
}
static void __exit my_exit (void)
{
    /* delete any running timers */
    printk (KERN_INFO "Deleted timer A: rc = %d\n", del_timer_sync (&timer_a));
    printk (KERN_INFO "Deleted timer B: rc = %d\n", del_timer_sync (&timer_b));
    kfree (data_a);
    kfree (data_b);
    printk (KERN_INFO "Module successfully unloaded \n");
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("CLDD:1.0 s_11/lab4_periodic_timers.c");
MODULE_LICENSE ("GPL v2");
