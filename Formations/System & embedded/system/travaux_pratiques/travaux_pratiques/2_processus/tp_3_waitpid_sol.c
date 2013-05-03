/*
 * Création répétée d'un processus fils et attente de sa fin, etc.
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SLEEP_TIME 1000

int main(void)
{
	pid_t pid;

	printf("Please kill pressing Ctrl + C\n");

	while (1) {
		if ((pid = fork()) == -1) {
			perror("fork");
			return EXIT_FAILURE;
		}

		if (pid == 0) {
			/* Processus fils */
			unsigned int random;

			fprintf(stderr, "One child born.\n");

			srand(time(NULL) % getpid());
			random = rand() % MAX_SLEEP_TIME;
			usleep(random);

			return EXIT_SUCCESS;
		}

		/* Processus parent */
		/* Purge les processus fils terminés */
		while (waitpid(-1, NULL, WNOHANG) > 0) {
			fprintf(stderr, "One child dead.\n");
		}
	}

	return EXIT_SUCCESS;
}
