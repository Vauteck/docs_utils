/*
 * Création de trois threads, deux incrémentent un compteur commun et le
 * troisième attendant que le compteur atteigne une valeur spécifique.
 */

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define COUNT_TOTAL 10
#define COUNT_THRESHOLD 12

int counter = 0;

/* Initialisation du mutex et de la variable conditionelle */
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t threshold_condv = PTHREAD_COND_INITIALIZER;

/*
 * Fonction exécutée par les threads "counter"
 *   Incrémente le compteur et signale la variable cond. si
 *   une valeur spécifique est atteinte
 */
void *inc_counter(void *thread_id)
{
	long th_id = (long) thread_id;

	for (int ii = 0; ii < COUNT_TOTAL; ii++) {
		/* Acquisition du mutex */
		pthread_mutex_lock(&counter_mutex);

		counter++;
		fprintf(stderr, "Counter thread %ld, count = %d\n", th_id, counter);

		if (counter == COUNT_THRESHOLD) {
			/* Signale la variable cond. */
			pthread_cond_signal(&threshold_condv);
			fprintf(stderr, "Counter thread %ld, threshold reached\n", th_id);
		}

		/* Libération du mutex */
		pthread_mutex_unlock(&counter_mutex);

		/* Laisse le temps aux threads d'alterner sur le mutex */
		sleep(1);
	}

	return NULL;
}


/*
 * Fonction exécutée par le thread "watcher"
 *   Attend le signal de la variable cond. pour agir sur le compteur.
 */
void *watch_counter(void *arg __attribute__ ((unused)))
{
	fprintf(stderr, "Starting watcher thread\n");

	/* Acquisition du mutex */
	pthread_mutex_lock(&counter_mutex);

	if (counter < COUNT_THRESHOLD) {
		/* Attente du signal de la varible cond. */
		pthread_cond_wait(&threshold_condv, &counter_mutex);

		fprintf(stderr, "Watcher thread, condition signal received\n");
		counter += 125;
		fprintf(stderr, "Watcher thread, count now = %d.\n", counter);
	}

	/* Libération du mutex */
	pthread_mutex_unlock(&counter_mutex);

	return NULL;
}

int main(void)
{
	pthread_t thread_ids[3];

	/* Création des threads */
	if (pthread_create(&thread_ids[0], NULL, &watch_counter, NULL) != 0) {
		perror("pthread_create 0");
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&thread_ids[1], NULL, &inc_counter, (long *) 1) != 0) {
		perror("pthread_create 1");
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&thread_ids[2], NULL, &inc_counter, (long *) 2) != 0) {
		perror("pthread_create 2");
		exit(EXIT_FAILURE);
	}

	/* Attente de la terminaison des threads */
	for (int ii = 0; ii < 3; ii ++) {
		if (pthread_join(thread_ids[ii], NULL) != 0) {
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
}
