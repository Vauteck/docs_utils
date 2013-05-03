/*
 * Programme avec deux processus, chaque processus affichant en
 * boucle un caractère différent.
 */

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

#define COUNT 500

int main(void)
{
	pid_t child;
	int count;

	/* Création d'un nouveau processus */
	if ((child = fork()) == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (child == 0) {
		/* Processus fils */
		for (count = 0; count <= COUNT; count++) {
			fputc('x', stderr);
		}
		exit(EXIT_SUCCESS);
	}

	/* Processus père */
	for (count = 0; count <= COUNT; count++) {
		fputc('o', stderr);
	}

	/* Attente de la terminaison du processus fils */
	if (wait(NULL) == -1) {
		perror("wait");
		exit(EXIT_FAILURE);
	}

	/* Juste pour avoir le prompt sur sa propre ligne */
	fputc('\n', stderr);

	exit(EXIT_SUCCESS);
}
