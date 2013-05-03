/************************************************************************\

  Equivalent d'un "cat" non-bloquant pour tester l'exemple 15

\************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


void cat_nonblock(int fd)
{
	char buffer[4096];
	int n;

	char * attente = "|/-\\";
	int i = 0;

	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

	while ((n = read(fd,buffer, 4096)) != 0) {
		if (n > 0) {
			write(STDOUT_FILENO, buffer, n);
			continue;
		}

		write(STDOUT_FILENO, & (attente[i]), 1);
		write(STDOUT_FILENO, "\b", 1);
		i ++;
		if (i == 4)
			i = 0;
		usleep(100000);
	}
}


int main(int argc, char * argv[])
{
	int i;
	int fd;
	
	for (i = 1; i < argc; i ++) {
		fd = open(argv[i], O_RDONLY, 0);
		if (fd < 0)
			continue;
		cat_nonblock(fd);
		close(fd);
	}
	if (argc == 1)
		cat_nonblock(STDIN_FILENO);
	return 0;
}
