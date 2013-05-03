/*
 * Serveur UDP en attente de recevoir un message pour le retourner à l'envoyeur.
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UDP_PORT 9090

int main(void)
{
	int sockfd;
	struct sockaddr_in src_addr, dest_addr;
	socklen_t addrlen = sizeof (struct sockaddr_in);
	int count;

	/* Ouvre une socket UDP */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* Adresse et port distants */
	memset(&dest_addr, 0, addrlen);
	dest_addr.sin_family = AF_INET;

	/* Adresse et port locaux */
	memset(&src_addr, 0, addrlen);
	src_addr.sin_family = AF_INET;
	src_addr.sin_port = htons(UDP_PORT);
	src_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Attache la socket à l'adresse locale */
	if (bind(sockfd, (struct sockaddr *) &src_addr, addrlen) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	while(1) {
		char msg[BUFSIZ];

		/* Reçoit un message */
		if ((count = recvfrom(sockfd, msg, BUFSIZ - 1, 0, (struct sockaddr *) &dest_addr, &addrlen)) < 0) {
			perror ("recvfrom");
			exit(EXIT_FAILURE);
		}
		msg[count] = 0;
		printf("received %s from %s\n", msg, inet_ntoa(dest_addr.sin_addr));

		/* Retourne le message */
		if (sendto(sockfd, msg, count, 0, (struct sockaddr *) &dest_addr, addrlen) < 0) {
			perror ("sendto");
			exit(EXIT_FAILURE);
		}
		printf("sent %s to %s:%d\n", msg, inet_ntoa(dest_addr.sin_addr), ntohs(dest_addr.sin_port));
	}
}
