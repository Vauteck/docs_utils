/*
 * Capture le signal SIGINT et reset son action par défaut
 * Le programme devra être de interrompu par un nouveau SIGINT.
 */

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

void handler(int sig)
{
	printf("Signal %d reçu\n", sig);

	/* Reset l'action par défaut du signal */
	if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
		fprintf(stderr, "signal error: SIG_DFL");
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	/* Assigne la fonction handler() au signal SIGINT */
	if (signal(SIGINT, handler) == SIG_ERR) {
		fprintf(stderr, "signal error: handler assignement");
		exit(EXIT_FAILURE);
	}

	/* Boucle indéfiniment */
	while (1) {
		printf("Hello world\n");
		sleep(1);
	}
}
