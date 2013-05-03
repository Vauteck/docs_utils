/*
 * Exemple d'utilisation des fonctions fchmod et unlink
 * Création d'un fichier vide, changement de ses permissions
 * et effacement du fichier
 * Entre chaque étape, la commande ls affiche les infos du fichier
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
	int fd;
	mode_t mode = 0755;

	/* Création d'un fichier vide */
	if (0 > (fd = open("empty.file", O_CREAT, 0644))) {
		perror("open");
		return EXIT_FAILURE;
	}

	/* Voyons les permissions du fichier */
	if (0 > system("ls -l empty.file")) {
		puts("system failed");
	}

	/* Changement des permissions */
	if (0 > fchmod(fd, mode)) {
		perror("fchmod");
		return EXIT_FAILURE;
	}

	/* Vérification du changement */
	if (0 > system("ls -l empty.file")) {
		puts("system failed");
	}

	close(fd);

	/* Effacement du fichier */
	unlink("empty.file");

	return EXIT_SUCCESS;
}
