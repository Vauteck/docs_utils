/*
 * Recherche les adresses IP d'un host avec gethostbyname()
 *
 */

#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	int i;
	struct hostent *host;
	struct in_addr **addr_list;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s hostname\n", argv[0]);
		return 1;
	}

	// Récupère les infos du host
	if ((host = gethostbyname(argv[1])) == NULL) {
		herror("gethostbyname");
		return 1;
	}

	// Récupère les infos du host
	printf("Official name is: %s\n", host->h_name);
	printf("    IP addresses: ");
	addr_list = (struct in_addr **) host->h_addr_list;
	for (i = 0; addr_list[i] != NULL; i++) {
		printf("%s ", inet_ntoa(*addr_list[i]));
	}
	printf("\n");

	return 0;
}
