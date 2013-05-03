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
	memset(&act, 0, sizeof(act));

	/*
	 * A completer :
	 * Assigner la fonction handler() pour un seul déclenchement
	 *
	 */

	/*
	 * A completer :
	 * Assigner la structure act au signal SIGINT
	 *
	 */

	/* Boucle indéfiniment */
	while (1) {
		printf("Hello world\n");
		sleep(1);
	}
}
