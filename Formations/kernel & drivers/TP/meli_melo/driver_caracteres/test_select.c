/************************************************************************\

  test_select

\************************************************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>

	#define LG_BUFFER 4096

	int
main (int argc, char * argv[])
{
	int            i, n, ret;
	fd_set         set;
	int  *         fd;
	struct timeval tv;
	time_t         heure;
	struct tm *    tm_heure;
	char buffer[LG_BUFFER];

	if (argc < 2) {
		fprintf(stderr, "usage: %s fichiers...\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	fd = calloc(argc - 1, sizeof(int));
	if (fd == NULL) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}
	
	n = 0;
	for (i = 0; i < argc - 1; i ++) {
		fd[i] = open(argv[1+i], O_RDONLY);
		if (fd[i] < 0) {
			perror(argv[1+i]);
			exit(EXIT_FAILURE);
		}
		if (fd[i] > n)
			n = fd[i];
	}
	
	while (1) {
		time(& heure);
		tm_heure = localtime(& heure);
		fprintf(stderr, "\r%02d:%02d:%02d ",
		        tm_heure->tm_hour, tm_heure->tm_min, tm_heure->tm_sec);
		
		FD_ZERO(& set);
		for (i = 0; i < argc-1; i ++)
			FD_SET(fd[i], & set);
		
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		
		ret = select (n+1, & set, NULL, NULL, & tv);
		if (ret < 0)
			break;
		if (ret == 0)
			continue;
		for (i = 0; i < argc-1; i ++) {
			if (! FD_ISSET(fd[i], & set))
				continue;
			ret = read(fd[i], buffer, LG_BUFFER);
			if (ret > 0)
				fprintf(stderr, "[%s] : %s\n", argv[1+i], buffer);
		}
	}
	return EXIT_SUCCESS;
}
