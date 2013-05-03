/*
 * Exemple d'un daemon simple
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	pid_t pid;

	/* Création d'un nouveau processus */
	if ((pid = fork()) == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		/* Processus père */
		exit(EXIT_SUCCESS);
	}

	/* Processus fils */

	/* Définition des permissions des nouveaux fichiers */
	umask(0);

	/* Obtention d'un nouvel identificateur de session */
	if (setsid() == -1) {
		perror("setsid");
		exit(EXIT_FAILURE);
	}

	/* Redirection des descripteurs d'E/S standards */
	if (freopen("/dev/null", "r", stdin) != stdin || \
			freopen("/dev/null", "w", stdout) != stdout || \
			freopen("/dev/null", "w", stderr) != stderr) {
		perror("freopen");
		exit(EXIT_FAILURE);
	}

	/* Change le répertoire courant */
	if (chdir("/") == -1) {
		perror("chdir");
		exit(EXIT_FAILURE);
	}

	sleep(90);
}
