/*
 * Copie un fichier dans un autre en utilisant les fonctions de <stdio.h>
 */

#include <stdio.h>
#include <stdlib.h>

#define FILE_IN "foo"
#define FILE_OUT "bar"

int main(void)
{
	FILE *in, *out;
	int c;

	/* Ouverture du fichier à copier, en mode lecture */
	if ((in = fopen(FILE_IN, "r")) == NULL) {
		perror("open in");
		exit(EXIT_FAILURE);
	}

	/* Création du fichier destinataire, en mode ecriture */
	if ((out = fopen(FILE_OUT, "w")) == NULL) {
		perror("open out");
		exit(EXIT_FAILURE);
	}

	/* Copie octet par octet d'un fichier dans l'autre */
	while ((c = fgetc(in)) != EOF) {
		if (fputc(c, out) == EOF) {
			fprintf(stderr, "fputc error");
			exit(EXIT_FAILURE);
		}
	}

	fclose(in);
	fclose(out);

	exit(EXIT_SUCCESS);
}
