#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>


int main(int argc, char * argv[])
{
	int fd;

	char * ptr;

	if ((argc != 2)
	 || ((fd = open(argv[1], O_RDWR, 0)) < 0)) {
		fprintf(stderr, "usage: %s peripherique\n", argv[0]);
		exit(1);
	}
	ptr = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
	if (ptr == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	while (1) {
		fprintf(stderr, "\r%s", ptr);
		usleep(100000); /* 1/10e s. */
	}

	munmap(ptr, 4096);
	close(fd);
	return 0;

}
