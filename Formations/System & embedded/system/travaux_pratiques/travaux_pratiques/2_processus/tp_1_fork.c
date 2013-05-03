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

	/*
	 * A completer :
	 * Création d'un nouveau processus
	 *
	 */
	child = ...

	/*
	 * A completer:
	 * Affichage des PID et PPID du processus fils
	 *
	 */
	if (child == ...)
		printf("in child\n");

	/*
	 * A completer :
	 * Affichage des PID et PPID du processus père
	 *
	 */
	printf("in parent\n");

	/*
	 * A completer :
	 * Attente de la terminaison du processus fils
	 *
	 */


	exit(EXIT_SUCCESS);
}
