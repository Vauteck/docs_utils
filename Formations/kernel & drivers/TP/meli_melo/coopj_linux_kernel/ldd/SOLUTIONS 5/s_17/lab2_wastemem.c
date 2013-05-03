/* **************** CLDD:1.0 s_17/lab2_wastemem.c **************** */
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

/* simple program to defragment memory, J. Cooperstein 2/04 
 @*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char **argv)
{
    int j;
    char *c;
    int m = atoi (argv[1]);
    c = malloc (m * 1024 * 1024);
    for (j = 0; j < m; j++) {
        memset (c + 1024 * 1024 * j, j, 1024 * 1024);
        printf ("%5d", j);
        fflush (stdout);
    }
    printf ("Quitting and releasing memory\n");
    exit (0);
}
