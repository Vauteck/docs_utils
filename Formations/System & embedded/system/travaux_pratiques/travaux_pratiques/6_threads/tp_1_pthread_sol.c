/*
 * Créé deux threads ; l'un affiche des 'o', l'autre des 'x'
 *   Une seule fonction est utilisée pour les deux threads, le caractère à
 *   afficher et le nombre de fois sont passés dans une stucture.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

struct char_print_parms
{
	char character;		/* Caractère à afficher */
	int count;			/* Nombre de caractères à afficher */
};


/*
 * Fonction exécutée par les threads
 *   La structure char_print_parms est passée dans *parameters
 */
void *char_print(void *parameters)
{
	struct char_print_parms *p = (struct char_print_parms *) parameters;

	for (int ii = 0; ii < p->count; ii++) {
		fputc(p->character, stderr);
	}

	return NULL;
}

int main(void)
{
	pthread_t thread1_id, thread2_id;
	struct char_print_parms thread1_args, thread2_args;

	/* Création d'un thread qui affiche des 'x' */
	thread1_args.character = 'x';
	thread1_args.count = 500;
	if (pthread_create(&thread1_id, NULL, &char_print, &thread1_args) != 0) {
		perror("pthread_create 1");
		exit(EXIT_FAILURE);
	}

	/* Création d'un thread qui affiche des 'o' */
	thread2_args.character = 'o';
	thread2_args.count = 700;
	if (pthread_create(&thread2_id, NULL, &char_print, &thread2_args) != 0) {
		perror("pthread_create 2");
		exit(EXIT_FAILURE);
	}

	/* Attente de la terminaison des threads */
	if (pthread_join(thread1_id, NULL) != 0) {
		perror("pthread_join 1");
		exit(EXIT_FAILURE);
	}
	if (pthread_join(thread2_id, NULL) != 0) {
		perror("pthread_join 2");
		exit(EXIT_FAILURE);
	}

	fputc('\n', stderr);

	exit(EXIT_SUCCESS);
}
