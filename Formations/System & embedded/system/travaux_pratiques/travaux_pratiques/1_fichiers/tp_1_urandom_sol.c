/*
 * Création d'un fichier de contenu alétoire dont la taille est spécifiée en
 * argument, en nombre de blocs de 1024 octets.
 *
 * La taille du fichier créé est volontairement limitée à 1 GB.
 */

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#define FILENAME "foo"
#define KBYTE 1024

int main(int argc, char **argv)
{
	long target;
	char *end;

	int fdin, fdout;
	char buf[KBYTE];
	ssize_t count, written = 0;

	/* Conversion de l'argument en entier */
	if (argc != 2) {
		fprintf(stderr, "USAGE: %s <number of 1KB blocks>\n", argv[0]);
		return EXIT_FAILURE;
	}
	errno = 0;
	target = strtoul(argv[1], &end, 10);
	if ((target == LONG_MIN || target == LONG_MAX) && errno != 0) {
		perror("strtol");
		return EXIT_FAILURE;
	}
	if (*end != '\0' || target <= 0 || target > (KBYTE * KBYTE)) {
		fprintf(stderr, "strtol: invalid number of blocks\n");
		return EXIT_FAILURE;
	}
	target *= KBYTE;

	/* Ouverture du device urandom, en mode lecture */
	if ((fdin = open("/dev/urandom", O_RDONLY)) < 0) {
		perror("open dev");
		exit(EXIT_FAILURE);
	}

	/* Création du fichier destinataire, en mode ecriture, droits d'acces 0600 */
	if ((fdout = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
		perror("open out");
		exit(EXIT_FAILURE);
	}

	/* Copie par bloc d'un fichier dans l'autre */
	while (written < (ssize_t) target) {
		if ((count = read(fdin, buf, KBYTE)) < 0) {
			perror("read in");
			exit(EXIT_FAILURE);
		}
		if ((count = write(fdout, buf, count)) < 0) {
			perror("write out");
			exit(EXIT_FAILURE);
		}
		written += count;
	}

	close(fdin);
	if (close(fdout) != 0) {
		perror("write out");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
