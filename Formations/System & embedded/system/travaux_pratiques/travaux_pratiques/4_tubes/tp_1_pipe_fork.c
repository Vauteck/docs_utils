/*
 * Ouvre un tube entre deux processus.
 */

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int fd[2];			/* descripteurs pour le tube */
	char buf[PIPE_BUF];
	int pid, len;

	/*
	 * A completer :
	 * Création d'un tube
	 *
	 */

	/* Création d'un processus */
	if (0 > (pid = fork())) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (0 == pid) {
		/* Processus fils */

		/*
		 * A completer :
		 * Lire des données depuis le tube et les afficher
		 *
		 */

		exit(EXIT_SUCCESS);
	}

	/* Processus père */

	/*
	 * A completer :
	 * Ecrire quelque données dans le tube
	 *
	 */


	/* Attente de la terminaison du processus fils */
	waitpid(pid, NULL, 0);

	exit(EXIT_SUCCESS);
}
