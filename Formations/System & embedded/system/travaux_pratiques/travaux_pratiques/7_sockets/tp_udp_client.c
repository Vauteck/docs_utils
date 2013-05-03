/*
 * Envoie et reçoit un message envoyé par un serveur UDP.
 *   Le message à envoyer est donné en argument.
 */

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UDP_PORT 9090
#define SRV_ADDR "127.0.0.1"

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in dest_addr;
	socklen_t addrlen = sizeof (struct sockaddr_in);
	char msg[BUFSIZ] = { 0 };
	int count;
	int len;

	if (argc != 2) {
		fprintf(stderr, "USAGE: %s <message>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	strncpy(msg, argv[1], BUFSIZ);
	msg[BUFSIZ - 1] = '\0';
	len = strlen(msg);

	/*
	 * A completer :
	 *   - ouvrir une socket UDP
	 *   - en utilisant comme adresse et port distants : SRV_ADDR et UDP_PORT
	 *   	- envoyer le message msg avec sendto()
	 *   	- recevoir le message retourné avec recvfrom()
	 *   - fermer la socket
	 */

	exit(EXIT_SUCCESS);
}
