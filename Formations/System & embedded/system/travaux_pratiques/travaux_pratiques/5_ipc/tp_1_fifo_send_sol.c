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

	/* Création de la clé */
	if ((key = ftok(argv[1], 0)) == -1) {
		perror("ftok");
		return EXIT_FAILURE;
	}

	/* Création de la file de messages */
	if ((fd = msgget(key, IPC_CREAT | 0600)) == -1) {
		perror("msgget");
		return EXIT_FAILURE;
	}

	/* Ecriture de argv[2] dans la file de messages */
	if (strncpy(message.text, argv[2], TEXTSIZE) == NULL) {
		perror("strncpy");
		return EXIT_FAILURE;
	}
	message.text[TEXTSIZE - 1] = '\0';
	message.type = MSG_TYPE;
	if (msgsnd(fd, (void *) &message, TEXTSIZE, 0) == -1) {
		perror("msgsnd");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
