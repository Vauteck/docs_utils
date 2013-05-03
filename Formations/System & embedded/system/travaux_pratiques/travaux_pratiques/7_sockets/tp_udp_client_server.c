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

#define UDP_PORT_1 9090
#define UDP_PORT_2 9091
#define IP_ADDR "127.0.0.1"

int main(int argc, char **argv)
{
	int fd1, fd2;
	struct sockaddr_in src_addr;
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

	if ((fd1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if ((fd1= socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	if ((fd2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* Adresse et port distants */
	memset(&dest_addr, 0, addrlen);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
	dest_addr.sin_port = htons(UDP_PORT_2);
	if (bind(fd2, (struct sockaddr *) &dest_addr, addrlen) < 0) {
		perror("bind 1");
		exit(EXIT_FAILURE);
	}

	/* Adresse et port locaux */
	src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	src_addr.sin_port = htons(UDP_PORT_1);
	if (bind(fd1, (struct sockaddr *) &src_addr, addrlen) < 0) {
		perror("bind 2");
		exit(EXIT_FAILURE);
	}

	/* Envoie le message */
	if (len != sendto(fd1, msg, len, 0, (struct sockaddr *) &dest_addr, addrlen)) {
		perror("sendto");
		exit(EXIT_FAILURE);
	}
	printf("sent %s to %s\n", msg, inet_ntoa(dest_addr.sin_addr));

	/* Reçoit un message */
	if ((count = recvfrom(fd2, msg, sizeof msg, 0, (struct sockaddr *) &dest_addr, &addrlen)) < 0) {
		perror("recvfrom");
		exit(EXIT_FAILURE);
	}
	printf("received %s from %s\n", msg, inet_ntoa(dest_addr.sin_addr));

	if (count != sendto(fd2, msg, count, 0, (struct sockaddr *) &dest_addr, addrlen))
	{
		perror("sendto");
		exit(EXIT_FAILURE);
	}
	printf("sent %s to %s:%d\n", msg, inet_ntoa(dest_addr.sin_addr), ntohs(dest_addr.sin_port));
	
	if ((count = recvfrom(fd1, msg, sizeof msg, 0, (struct sockaddr *) &dest_addr, &addrlen)) < 0) {
		perror("recvfrom");
		exit(EXIT_FAILURE);
	}
	printf("received %s from %s\n", msg, inet_ntoa(dest_addr.sin_addr));

	/* Ferme les sockets */
	close(fd1);
	close(fd2);

	exit(EXIT_SUCCESS);
}
