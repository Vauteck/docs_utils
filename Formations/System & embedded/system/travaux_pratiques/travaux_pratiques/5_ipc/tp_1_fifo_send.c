/*
 * Ecrit dans une file de messages.
 *   La clé et le message sont passés en argument au programme.
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

	/* Lecture des arguments du programme */
	if (argc != 3) {
		fprintf(stderr, "USAGE: %s <key file> <message>\n", argv[0]);
		return EXIT_FAILURE;
	}

	/*
	 * A completer :
	 * Création de la clé avec argv[1]
	 * Création de la file de messages
	 * Ecriture de arvg[2] dans la file
	 *
	 */

	return EXIT_SUCCESS;
}
