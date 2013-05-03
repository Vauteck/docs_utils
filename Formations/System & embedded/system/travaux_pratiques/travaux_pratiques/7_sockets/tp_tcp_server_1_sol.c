/*
 * Serveur TCP affichant les données reçues de client.
 *   Utilise select().
 *   Pour faire un test : pointer un navigateur web sur http://localhost:8080/
 */

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define RECV_PORT 8080

int main(void)
{
	int val;
	int sockfd;
	struct sockaddr_in src_addr, dest_addr;
	socklen_t addrlen = sizeof (struct sockaddr_in);

	/* Adresse et port distant */
	memset(&dest_addr, 0, addrlen);
	dest_addr.sin_family = AF_INET;

	/* Adresse et port locaux */
	memset(&src_addr, 0, addrlen);
	src_addr.sin_family = AF_INET;
	src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	src_addr.sin_port = htons(RECV_PORT);

	/* Ouvre une socket TCP */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	val = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *) &val, sizeof(val)) < 0 ) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	/* Attache la socket */
	if (bind(sockfd, (struct sockaddr *) &src_addr, addrlen) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	/* Definit la longueur de la queue */
	if (listen(sockfd, 5) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	while (1) {
		int new_sockfd;
		char buf[BUFSIZ];

		/* Accepte une nouvelle connection avec accept() */

		if ((new_sockfd = accept(sockfd, (struct sockaddr *) &dest_addr, &addrlen)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		/* Lit et affiche les donnees recues sur la nouvelle socket */
		if (read(new_sockfd, buf, BUFSIZ) > 0) {
			printf ("%s\n", buf);
		}

		close(new_sockfd);
	}
}
