/*
 * Redirige la sortie standard vers un fichier nouvellement créé et
 * exécute la commande ls
 * Le résultat de la commande ls se trouve donc copié dans le fichier
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#define FILENAME "dup.out"

int main(void)
{
	int fd;

	/* Création du fichier en mode écriture, avec droits 644 */
	if ((fd = open(FILENAME, O_RDWR | O_CREAT | O_TRUNC, 0644)) < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	/*
	 * A completer :
	 * Fermeture de la sortie standard
	 *
	 */

	/*
	 * A completer :
	 * Duplication du descripteur fd
	 *
	 */

	/*
	 * A complter :
	 * Fermeture du descripteur fd
	 *
	 */

	/* Exécution de la commande ls */
	execlp("ls", "ls", NULL);
	perror("execlp");
	exit(EXIT_FAILURE);
}
