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

	/* Création de la clé */
	if ((key = ftok(argv[1], 0)) == -1) {
		perror("ftok");
		return EXIT_FAILURE;
	}

	/* Récupération de la file de messages */
	if ((fd = msgget(key, IPC_CREAT | 0600)) == -1) {
		perror("msgget");
		return EXIT_FAILURE;
	}

	/* Lecture dans la file de messages */
	if ((count = msgrcv(fd, (void *) &message, TEXTSIZE, MSG_TYPE, 0)) == -1) {
		perror("msgrcv");
		return EXIT_FAILURE;
	}
	message.text[TEXTSIZE - 1] = '\0';
	printf("(%ld) %s (%d)\n", message.type, message.text, count);

	/* Destruction de la file de messages */
	if (msgctl(fd, IPC_RMID, NULL)) {
		perror("msgctl");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
