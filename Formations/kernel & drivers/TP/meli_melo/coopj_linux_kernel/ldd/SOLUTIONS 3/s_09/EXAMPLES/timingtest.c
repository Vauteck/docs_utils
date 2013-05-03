/* **************** LDD:1.0 s_09/timingtest.c **************** */
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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* these headers are defined for x86_64, but not i386 */

/*
#if defined __i386__
#define rdtsc(low,high) \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#define rdtscl(low) \
     __asm__ __volatile__("rdtsc" : "=a" (low) : : "edx")
#define rdtscll(val) \
     __asm__ __volatile__("rdtsc" : "=A" (val))
#elif defined __x86_64__
#include <asm/msr.h>
#endif
*/

#define rdtsc(low,high) \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#define rdtscl(low) \
     __asm__ __volatile__("rdtsc" : "=a" (low) : : "edx")
#define rdtscll(val) \
     __asm__ __volatile__("rdtsc" : "=A" (val))

#define MHZ 1000000
#define NLOOP 10

int main (int argc, char *argv[])
{
    int j;
    unsigned long long start, end, diff;
    double avg = 0;

    for (j = 0; j < NLOOP; j++) {
        rdtscll (start);
        sleep (1);
        rdtscll (end);
        diff = end - start;
        avg += diff;
        printf (" start = %lld  end = %lld  elapsed = %lld  mhz = %lld\n",
                start, end, diff, diff / MHZ);
    }
    printf (" Average = %f\n", avg / (double)(MHZ * NLOOP));
    exit (0);
}
