/*
 * Exemple d'utilisation de la fonstion ioctl
 * Ouvre le tiroir d'un lecteur de CD
 * Le path du device est donné en argument (ex : /dev/cdrom)
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char **argv)
{
	int fd;

	if (argc != 2) {
		fprintf(stderr, "USAGE: %s <cdrom device path>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (0 > (fd = open(argv[1], O_RDONLY | O_NONBLOCK))) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	if (0 > (ioctl(fd, CDROMEJECT, 0))) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
