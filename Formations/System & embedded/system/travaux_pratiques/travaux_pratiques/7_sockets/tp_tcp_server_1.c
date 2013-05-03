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


	/*
	 * A completer ;
	 *   - ouvrir une socket TCP avec l'option SO_REUSEADDR
	 *   - attacher la socket à l'adresse INADDR_ANY et au port RECV_PORT
	 *   - définir une queue de 5 clients
	 */
	 
	while (1) {
		int new_sockfd;
		char buf[BUFSIZ];

		/*
		 * A completer 
		 *   - accepter une nouvelle connection avec accept()
		 *   - lire et afficher les données reçues
		 */
	}
}
