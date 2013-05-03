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

	/*
	 * A completer : récupèrer les infos du host argv[1] avec gethostbyname()
	 */

	printf("Host name : %s\n", host->h_name);

	/*
	 * A completer : afficher l'adresse IP du host
	 */
	printf("IP Address: %s\n", ...

   return 0;
}
