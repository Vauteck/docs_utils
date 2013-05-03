/*
 * Création de deux threads incrémentant une variable protégée par un mutex
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

#define NTHREADS 10

int counter = 4;

sem_t semaphore;

/*
 * Fonction exécutée par les threads
 */
void *inc_counter(void *arg)
{
	(void) arg;

	/*
	 * A completer :
	 * Acquisition du semaphore
	 * Incrémentation de counter
	 *
	 */

	fprintf(stderr, "Thread %ld : Counter value: %d\n", pthread_self(), counter);

	/*
	 * A completer
	 * Libération du semaphore
	 *
	 */

	return NULL;
}


int main(void)
{
	pthread_t thread_id[NTHREADS];
	int ii;

	/*
	 * A completer :
	 * Initialisation du semaphore
	 * Créer NTHREADS
	 * Attendre la terminaison des threads
	 * Destruction du semaphore
	 *
	 */

	printf("Final counter value %d\n", counter);

	exit(EXIT_SUCCESS);
}
