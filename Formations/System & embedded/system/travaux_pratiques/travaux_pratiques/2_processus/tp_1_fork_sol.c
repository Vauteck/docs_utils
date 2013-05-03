/*
 * Creer un processus avec fork et affiche les PIDs et PPIDs du parent et du fils.
 */

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	pid_t child;

	/* Création d'un nouveau processus */
	if ((child = fork()) == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (child == 0) {
		/* Affichage des PID et PPID du processus fils */
		printf("in child\n");
		printf("child pid = %d\n", getpid());
		printf("child ppid = %d\n", getppid());
		exit(EXIT_SUCCESS);
	} else {
		/* Affichage des PID et PPID du processus père */
		printf("in parent\n");
		printf("parent pid = %d\n", getpid());
		printf("parent ppid = %d\n", getppid());
		/* Attente de la terminaison du processus fils */
		if (wait(NULL) == -1) {
			perror("wait");
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
}
