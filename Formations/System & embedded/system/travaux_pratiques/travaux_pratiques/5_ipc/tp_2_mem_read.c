/*
 * Lit un segment de mémoire partagée protégé par un sémaphore.
 *   La clé du sémaphore est donné en argument du programme.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

#define MSGSIZE 0xFF

int main(int argc, char **argv)
{
	key_t key;
	int sem;
	int shm;
	struct sembuf sembuf;
	char *message = NULL;

	if (argc != 2) {
		fprintf(stderr, "USAGE: %s <key file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/*
	 * A completer :
	 * Création de la clé avec argv[1]
	 * Récupération du sémaphore
	 * Récupération du segment de mémoire
	 * Attachement au segment de mémoire
	 * Attente du sémaphore
	 * Affichage du message
	 * Libération du sémaphore
	 * Destruction de segment de mémoire
	 * Destruction du sémaphore
	 *
	 */

	exit(EXIT_SUCCESS);
}

