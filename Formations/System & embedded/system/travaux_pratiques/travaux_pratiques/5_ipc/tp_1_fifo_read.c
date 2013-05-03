/*
 * Lit depuis une file de messages.
 *   La clé est passée en argument au programme.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define  MSG_TYPE 0x1234L

#define  TEXTSIZE 0xFF

typedef struct {
	long type;
	char text[TEXTSIZE];
} msg_t;

int main(int argc, char **argv)
{
	key_t key;
	msg_t message;
	int fd;
	int count;

	/* Lecture des arguments du programme */
	if (argc != 2) {
		fprintf(stderr, "USAGE: %s <key file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	/*
	 * A completer :
	 * Création de la clé avec argv[1]
	 * Récupération de la file de messages
	 * Lecture dans la file de messages
	 * Optionel : destruction de la file de messages
	 *
	 */

	return EXIT_SUCCESS;
}
