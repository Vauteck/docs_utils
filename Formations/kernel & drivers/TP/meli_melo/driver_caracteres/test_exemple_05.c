/************************************************************************\
  
  Fonction d'ioctl

\************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "exemple_05.h"

int main(int argc, char * argv[])
{
	int fd;
	int affiche_ppid;

	if (argc < 2) {
		fprintf(stderr, "usage: %s fichier [affiche_ppid]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	fd = open(argv[1], O_RDONLY, 0);
	if (fd < 0) {
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}
	if (argc == 2) {
		if (ioctl(fd, EX_IOCGPPID, & affiche_ppid) != 0) {
			perror("ioctl");
			exit(EXIT_FAILURE);
		} 
		fprintf(stdout, "%d\n", affiche_ppid);
		exit(EXIT_SUCCESS);
	}
	if (sscanf(argv[2], "%d", & affiche_ppid) != 1) {
		fprintf(stderr, "%s : valeur invalide\n", argv[2]);
		exit(EXIT_FAILURE);
	}
	if (ioctl(fd, EX_IOCSPPID, & affiche_ppid) != 0) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	} 
	return 0;
}

