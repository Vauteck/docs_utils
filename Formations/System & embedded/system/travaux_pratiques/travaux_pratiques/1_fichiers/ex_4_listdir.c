/*
 * Exemple d'utilisation de la fonction opendir
 * Liste le contenu d'un répertoire donné en argument au programme
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char **argv)
{
	DIR *dir;
	struct dirent *mydirent;
	int ii = 1;

	if (argc != 2) {
		fprintf(stderr, "USAGE: %s PATHNAME\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (NULL == (dir = opendir(argv[1]))) {
		perror("opendir");
		return EXIT_FAILURE;
	}

	while (NULL != (mydirent = readdir(dir))) {
		printf("%3d : %s\n", ii++, mydirent->d_name);
	}

	closedir(dir);

	return EXIT_SUCCESS;
}
