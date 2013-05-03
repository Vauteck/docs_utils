/*
 * Créér deux threads ; l'un affiche des 'o', l'autre des 'x'
 *   Une seule fonction sera utilisée pour les deux threads, le caractère à
 *   afficher et le nombre de fois seront passés dans une stucture.
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
	/*
	 * A completer :
	 * Faire un cast pour récupérer la structure
	 *
	 */

	for (int ii = 0; ii < p->count; ii++) {
		fputc(p->character, stderr);
	}

	return NULL;
}

int main(void)
{
	pthread_t thread1_id, thread2_id;
	struct char_print_parms thread1_args, thread2_args;

	/*
	 * A completer :
	 * Création d'un thread qui affichera 500 'x'
	 * Création d'un thread qui affichera 700 'o'
	 * Attente de la terminaison des threads
	 *
	 */

	exit(EXIT_SUCCESS);
}
