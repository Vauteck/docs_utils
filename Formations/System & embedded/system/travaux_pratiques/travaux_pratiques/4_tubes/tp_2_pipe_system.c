/*
 * Pipe la sortie d'un processus vers l'entrée d'un autre
 *   On souhaite simuler ls | sort
 *   Le processus père invoquera la commande sort
 *   Le processus fils invoquera la commande ls
 *   Un tube servira pour fournir la sortie de ls à l'entrée de sort
 */

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main (void)
{
	int fd[2];			/* descripteurs pour le tube */
	int pid;
	int ret;

	/* Création du tube */
	if ((pipe(fd)) < 0) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	if ((pid = fork()) == -1) {
		perror("fork()");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
		/* Processus fils */

		/*
		 * A completer :
		 * Connecter l'entrée du tube avec la sortie de la commande ls
		 *
		 */

		/* Invoque une commande */
		ret = system("ls");

		exit(EXIT_SUCCESS);
	}


	/* Processus père */

	/*
	 * A completer :
	 * Connecter la sortie du tube avec l'entrée de la commande sort
	 */

	/* Invoque une commande */
	ret = system("sort");

	/* Attente du fils */
	wait(NULL);

	exit(EXIT_SUCCESS);
}
