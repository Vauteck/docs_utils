/*
 * Demo de select() sur l'entrée standard
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
	struct timeval tv;
	fd_set readfds;


	/*
	 * A completer :
	 *  - remplir readfds avec STDIN_FILENO
	 *  - utiliser select() sur readfs avec un timeout de 2.5 secondes
	 *  - lire le carctère lu
	 */

	return 0;
}

