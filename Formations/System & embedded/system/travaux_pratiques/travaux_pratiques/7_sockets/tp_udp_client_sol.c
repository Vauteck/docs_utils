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
//	struct sockaddr_in src_addr;
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

	/* Ouvre une socket UDP */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* Adresse et port distants */
	memset(&dest_addr, 0, addrlen);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(SRV_ADDR);
	dest_addr.sin_port = htons(UDP_PORT);

	/* Adresse et port locaux
	memset(&src_addr, 0, addrlen);
	src_addr.sin_family = AF_INET;
	src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	src_addr.sin_port = htons(0);
	*/
	/* Attache la socket à l'adresse locale
	if (bind(sockfd, (struct sockaddr *) &src_addr, addrlen) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	*/

	/* Envoie le message */
	if (len != sendto(sockfd, msg, len, 0, (struct sockaddr *) &dest_addr, addrlen)) {
		perror("sendto");
		exit(EXIT_FAILURE);
	}
	printf("sent %s to %s\n", msg, inet_ntoa(dest_addr.sin_addr));

	/* Reçoit un message */
	if ((count = recvfrom(sockfd, msg, sizeof msg, 0, (struct sockaddr *) &dest_addr, &addrlen)) < 0) {
		perror("recvfrom");
		exit(EXIT_FAILURE);
	}
	printf("received %s from %s\n", msg, inet_ntoa(dest_addr.sin_addr));

	/* Ferme la socket */
	close(sockfd);

	exit(EXIT_SUCCESS);
}
