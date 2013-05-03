/*
 * Ouvre un tube entre deux processus.
 */

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSZ PIPE_BUF

int main(void)
{
	int fd[2];			/* descripteurs pour le tube */
	char buf[PIPE_BUF];
	int pid, len;

	/* Création du tube */
	if ((pipe(fd)) < 0) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	/* Création d'un processus */
	if (0 > (pid = fork())) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if (0 == pid) {
		/* Processus fils */

		/* Fermeture du descripteur d'écriture */
		close(fd[1]);

		/* Lecture depuis le tube et affichage sur la sortie standard */
		while (0 < (len = read(fd[0], buf, PIPE_BUF - 1))) {
			buf[len] = '\0';
			printf("Lecture :\n%s\n", buf);
		}

		/* Fermeture du descripteur de lecture */
		close(fd[0]);

		exit(EXIT_SUCCESS);
	}

	/* Processus père */

	/* Fermeture du descripteur de lecture */
	close(fd[0]);

	/* Ecrit quelque chose dans le tube */
	len = write(fd[1], "123\n456\n", 8);

	/* Fermeture du descripteur d'écriture */
	close(fd[1]);

	/* Attente de la terminaison du processus fils */
	waitpid(pid, NULL, 0);

	exit(EXIT_SUCCESS);
}
