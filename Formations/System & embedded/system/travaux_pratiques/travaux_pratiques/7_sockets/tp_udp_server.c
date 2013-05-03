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

	/*
	 * A completer :
	 *   - Ouvrir une socket UDP
	 *   - attacher la socket avec bind() à l'adresse INADDR_ANY et au port UDP_PORT
	 */

	while(1) {
		char msg[BUFSIZ];

		/*
		 * A completer : reçevoit un message et le retourner à l'envoyeur
		 */
	}
}
