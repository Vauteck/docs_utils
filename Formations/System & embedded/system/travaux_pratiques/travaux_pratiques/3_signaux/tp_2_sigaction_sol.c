/*
 * Capture le signal SIGINT une seule fois.
 * Utilise sigaction().
 */

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void handler(int sig)
{
	printf("Signal %d reçu\n", sig);
}

int main(void)
{
	struct sigaction act;

	/* Assigne la fonction handler() pour un seul déclenchement */
	memset(&act, 0, sizeof(act));
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_ONESHOT;

	/* Assigne la structure act au signal SIGINT */
	if (sigaction(SIGINT, &act, 0) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	/* Boucle indéfiniment */
	while (1) {
		printf("Hello world\n");
		sleep(1);
	}
}
