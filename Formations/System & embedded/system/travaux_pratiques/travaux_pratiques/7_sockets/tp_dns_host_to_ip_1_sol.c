/*
 * Recherche l'adresse IP d'un host avec gethostbyname()
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	struct hostent *host;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s address\n", argv[0]);
		return 1;
	}

	// Récupère les infos du host
	if ((host = gethostbyname(argv[1])) == NULL) {
		herror("gethostbyname");
		return 1;
	}

	printf("Host name : %s\n", host->h_name);
	printf("IP Address: %s\n", inet_ntoa(*((struct in_addr *) host->h_addr)));

   return 0;
}
