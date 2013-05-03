/*
 * Création de deux threads incrémentant une variable protégée par un mutex
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define NTHREADS 10

int counter = 4;

/* Initialisation du mutex */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Fonction exécutée par les threads
 */
void *inc_counter(void *arg)
{
	(void) arg;

	/* Acquisition du mutex */
	pthread_mutex_lock(&mutex);

	counter++;
	fprintf(stderr, "Thread %ld : Counter value: %d\n", pthread_self(), counter);

	/* Libération du mutex */
	pthread_mutex_unlock(&mutex);

	return NULL;
}


int main(void)
{
	pthread_t thread_id[NTHREADS];
	int ii;

	/* Création des threads */
	for (ii = 0; ii < NTHREADS; ii ++) {
		if (pthread_create(&thread_id[ii], NULL, &inc_counter, NULL) != 0) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}

	/* Attente de la terminaison des threads */
	for (ii = 0; ii < NTHREADS; ii ++) {
		if (pthread_join(thread_id[ii], NULL) != 0) {
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}
	}

	printf("Final counter value %d\n", counter);

	exit(EXIT_SUCCESS);
}
