/* **************** "":1.0 s_33/lab1_posix_test.c **************** */
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
 *   (Posix API test program)
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
#include <aio.h>
#include <sys/stat.h>

#define NBYTES 32
#define NBUF   512

void printbufs(char **buf, int nbytes)
{
	int j;
	fflush(stdout);
	for (j = 0; j < NBUF; j++) {
		write(STDOUT_FILENO, buf[j], nbytes);
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	int fd, rc, j, k, nbytes = NBYTES;
	char *tmp, *buf[NBUF], *filename = "/dev/mycdrv";
	struct aiocb *cbray[NBUF], *cb;

	/* create the file and fill it with a pattern */

	if (argc > 1)
		filename = argv[1];

	/* notice opening with these flags won't hurt a device node! */

	if ((fd = open(filename, O_RDWR | O_CREAT | O_TRUNC,
		       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0) {
		printf("couldn't open %s, ABORTING\n", filename);
		exit(-1);
	}

	/* write initial data out, clear buffers, allocate aiocb's */

	for (j = 0; j < NBUF; j++) {

		buf[j] = malloc(nbytes);
		sprintf(buf[j], "%4d%4d%4d%4d%4d%4d%4d%4d", j, j, j, j, j, j, j,
			j);
		write(fd, buf[j], nbytes);
		memset(buf[j], 0, nbytes);

		cb = malloc(sizeof(struct aiocb));
		cbray[j] = cb;
		memset(cb, 0, sizeof(struct aiocb));

		cb->aio_fildes = fd;
		cb->aio_nbytes = nbytes;
		cb->aio_offset = j * nbytes;
		cb->aio_buf = (void *)buf[j];
	}
	printf("\n");

	/* (async) read the data from the file */

	printf(" reading initial data from the file:\n");

	for (j = 0; j < NBUF; j++) {
		cb = cbray[j];
		rc = aio_read(cb);
	}

	/* sync up and print out the readin data */

	for (j = 0; j < NBUF; j++) {
		cb = cbray[j];
		while (aio_error(cb) == EINPROGRESS) {
		};
		printf("%d:  aio_error=%d  aio_return=%d\n  ",
		       j, aio_error(cb), (int)aio_return(cb));
	}

	printbufs(buf, nbytes);

	/* filling in the buffers before the write */

	for (j = 0; j < NBUF; j++) {
		tmp = buf[j];
		for (k = 0; k < nbytes; k++) {
			sprintf((tmp + k), "%1d", j);
		}
		/*        printf ("%d, %s\n", j, buf[j]); */
	}
	printf("\n");

	/* write the changed buffers out */

	printf(" writing new data to the file:\n");
	for (j = 0; j < NBUF; j++) {
		cb = cbray[j];
		rc = aio_write(cb);
	}

	/* sync up again */

	for (j = 0; j < NBUF; j++) {
		cb = cbray[j];
		while (aio_error(cb) == EINPROGRESS) {
		};
		printf("%d:  aio_error=%d  aio_return=%d\n  ",
		       j, aio_error(cb), (int)aio_return(cb));
	}

	printbufs(buf, nbytes);

	close(fd);
	exit(0);
}
