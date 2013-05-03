/* **************** LDD:1.0 s_15/lab2_wastemem.c **************** */
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
/* simple program to defragment memory, J. Cooperstein 2/04 
 @*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MB (1024*1024)

int main (int argc, char **argv)
{
    int j;
    char *c;
    int m = atoi (argv[1]);
    for (j = 0; j < m; j++) {
        /* yes we know this is a memory leak, no free, that's the idea! */
        c = malloc (MB);
        memset (c, j, MB);
        printf ("%5d", j);
        fflush (stdout);
    }
    printf("All memory allocated, pausing 5 seconds\n");
    sleep(5);
    printf ("Quitting and releasing memory\n");
    exit (0);
}
