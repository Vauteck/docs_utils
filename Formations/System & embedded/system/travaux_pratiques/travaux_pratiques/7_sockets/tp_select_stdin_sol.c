/*
 * Demo de select sur l'entrée standard
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
	struct timeval tv;
	fd_set readfds;

	tv.tv_sec = 2;
	tv.tv_usec = 500000;

	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);

	select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

	if (FD_ISSET(STDIN_FILENO, &readfds)) {
		int key = getchar();
		printf("The key '%c' was pressed\n", key);
	} else {
		printf("Timed out.\n");
	}

	return 0;
}

