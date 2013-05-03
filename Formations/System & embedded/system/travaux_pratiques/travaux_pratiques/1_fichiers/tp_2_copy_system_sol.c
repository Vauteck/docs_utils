/*
 * Copie un fichier dans un autre en utilisant les fonctions systèmes.
 * La taille du bloc copié est donnée en argument au programme.
 */

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#define FILE_IN "foo"
#define FILE_OUT "bar"

#define MAX_BLOCK_SIZE 4096

int main(int argc, char **argv)
{
	char *end;
	long block_size;
	int fdin, fdout;
	char buf[MAX_BLOCK_SIZE];
	size_t nread;

	/* Conversion de l'argument en entier */
	if (argc != 2) {
		fprintf(stderr, "USAGE: %s <size of block>\n", argv[0]);
		return EXIT_FAILURE;
	}
	errno = 0;
	block_size = strtoul(argv[1], &end, 10);
	if ((block_size == LONG_MIN || block_size == LONG_MAX) && errno != 0) {
		perror("strtol");
		return EXIT_FAILURE;
	}
	if (*end != '\0' || block_size <= 0 || block_size > MAX_BLOCK_SIZE) {
		fprintf(stderr, "strtol: invalid number of blocks\n");
		return EXIT_FAILURE;
	}

	/* Ouverture du fichier à copier, en mode lecture */
	if ((fdin = open(FILE_IN, O_RDONLY)) < 0) {
		perror("open in");
		exit(EXIT_FAILURE);
	}

	/* Création du fichier destinataire, en mode ecriture (droits d'access 0600) */
	if ((fdout = open(FILE_OUT, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR|S_IWUSR)) < 0) {
		perror("open out");
		exit(EXIT_FAILURE);
	}

	/* Copie bloc par bloc d'un fichier dans l'autre */
	while ((nread = read(fdin, buf, (size_t) block_size)) > 0) {
		if (write(fdout, buf, nread) < 0) {
			perror("write out");
			exit(EXIT_FAILURE);
		}
	}

	close(fdin);
	if (close(fdout) != 0) {
		perror("close out");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
