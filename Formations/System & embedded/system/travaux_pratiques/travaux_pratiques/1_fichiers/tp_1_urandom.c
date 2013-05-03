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

	/*
	 * A completer :
	 *
	 * Ouverture du device urandom, en mode lecture seule.
	 *
	 */

	/*
	 * A Completer :
	 *
	 * Ouverture du fichier destinataire, en mode ecriture
	 * - créer le fichier s'il n'existe pas, le tronquer sinon)
	 * - droits d'acces 0600
	 *
	 */

	while (written < (ssize_t) target) {
		/*
		 * A completer :
		 *
		 * Copie par bloc d'un descripteur de fichier dans l'autre
		 *
		 */

		written += count;
	}

	/*
	 * A completer :
	 *
	 * Fermeture des descripteurs de fichier
	 *
	 */

	exit(EXIT_SUCCESS);
}
