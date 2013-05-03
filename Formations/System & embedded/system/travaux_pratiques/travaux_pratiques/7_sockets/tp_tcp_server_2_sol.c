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

	fd_set set;
	FD_ZERO(&set);
	FD_SET(sockfd, &set);
	while (1) {
		/* Attente d'une connection */
		if (select(FD_SETSIZE, &set, NULL, NULL, 0) < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				continue;
			}
			perror("select");
			exit(EXIT_FAILURE);
		}

		/* Cherche quels descripteurs sont connectés */
		for (int ii = 0; ii < FD_SETSIZE; ii++) {
			if (FD_ISSET(ii, &set)) {
				if (ii == sockfd) {
					/* Nouvelle connexion sur la socket originale */
					int newfd;
					if ((newfd = accept(sockfd, (struct sockaddr *) &dest_addr, &addrlen)) < 0) {
						perror("accept");
						exit(EXIT_FAILURE);
					}
					/* Ajoute cette connexion dans la liste des descripteurs surveillés */
					if (newfd < FD_SETSIZE) {
						FD_SET(newfd, &set);
					} else {
						fprintf(stderr, "accept: got fd >= FD_SETSIZE\n");
					}
				} else {
					/* Affiche les données arrivant sur une socket déjà connectée */
					char buf[BUFSIZ];
					int count;
					if ((count = read(ii, buf, BUFSIZ)) < 0) {
						perror("accept");
						exit(EXIT_FAILURE);
					}
					if (count != 0) {
						fprintf(stderr, "Server: got message:\n%s\n", buf);
					}
					close(ii);
					FD_CLR(ii, &set);
				}
			}
		}
	}
}
