/* **************** LDD:1.0 s_33/lab1_aio_test.c **************** */
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
 * Adding Asynchronous Entry Points to a Character Driver
 *
 *   (Linux API test program)
 *
 * Take one of your earlier character drivers and add new entry points
 * for aio_read() and aio_write().
 *
 * To test this you'll need to write a user-space program that uses
 * the native Linux API.  Have it send out a number of write and read
 * requests and synchronize properly.
 *
 * We also present a solution using the Posix API for the user
 * application; note that this will never hit your driver unless you
 * comment out the normal read and write entry points in which case
 * the kernel will fall back on the asynchronous ones.
 *
 * Make sure you compile by linking with the right libraries; use
 * -laio for the Linux API and -lrt for the Posix API.  (You can use
 * both in either case as they don't conflict.)
 @*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <libaio.h>
#include <sys/stat.h>

#define NBYTES 32
#define NBUF   100

void printbufs (char **buf, int nbytes)
{
    int j;
    fflush (stdout);
    for (j = 0; j < NBUF; j++) {
        write (STDOUT_FILENO, buf[j], nbytes);
        printf ("\n");
    }
    printf ("\n");
}

int main (int argc, char *argv[])
{
    int fd, rc, j, k, nbytes = NBYTES, maxevents = NBUF;
    char *buf[NBUF], *filename = "/dev/mycdrv";
    struct iocb *iocbray[NBUF], *iocb;
    off_t offset;
    io_context_t ctx = 0;
    struct io_event events[2 * NBUF];
    struct timespec timeout = { 0, 0 };

    /* open or create the file and fill it with a pattern */

    if (argc > 1)
        filename = argv[1];

    printf ("opening %s\n", filename);

    /* notice opening with these flags won't hurt a device node! */

    if ((fd = open (filename, O_RDWR | O_CREAT | O_TRUNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0) {
        printf ("couldn't open %s, ABORTING\n", filename);
        exit (-1);
    }

    /* write initial data out, clear buffers, allocate iocb's */

    for (j = 0; j < NBUF; j++) {
        /* no need to zero iocbs; will be done in io_prep_pread */
        iocbray[j] = malloc (sizeof (struct iocb));
        buf[j] = malloc (nbytes);
        sprintf (buf[j], "%4d%4d%4d%4d%4d%4d%4d%4d", j, j, j, j, j, j, j, j);
        write (fd, buf[j], nbytes);
        memset (buf[j], 0, nbytes);
    }
    printf ("\n");

    /* prepare the context */

    rc = io_setup (maxevents, &ctx);
    printf (" rc from io_setup = %d\n", rc);

    /* (async) read the data from the file */

    printf (" reading initial data from the file:\n");

    for (j = 0; j < NBUF; j++) {
        iocb = iocbray[j];
        offset = j * nbytes;
        io_prep_pread (iocb, fd, (void *)buf[j], nbytes, offset);
        rc = io_submit (ctx, 1, &iocb);
    }

    /* sync up and print out the readin data */

    while ((rc = io_getevents (ctx, NBUF, NBUF, events, &timeout)) > 0) {
        printf (" rc from io_getevents on the read = %d\n\n", rc);
    }

    printbufs (buf, nbytes);

    /* filling in the buffers before the write */

    for (j = 0; j < NBUF; j++) {
        char *tmp = buf[j];
        for (k = 0; k < nbytes; k++) {
            sprintf ((tmp + k), "%1d", j);
        }
    }

    /* write the changed buffers out */

    printf (" writing new data to the file:\n");
    for (j = 0; j < NBUF; j++) {
        iocb = iocbray[j];
        offset = j * nbytes;
        io_prep_pwrite (iocb, fd, buf[j], nbytes, offset);
        rc = io_submit (ctx, 1, &iocb);
    }

    /* sync up again */

    while ((rc = io_getevents (ctx, NBUF, NBUF, events, &timeout)) > 0) {
        printf (" rc from io_getevents on the write = %d\n\n", rc);
    }

    printbufs (buf, nbytes);

    /* clean up */
    rc = io_destroy (ctx);
    close (fd);
    exit (0);
}
