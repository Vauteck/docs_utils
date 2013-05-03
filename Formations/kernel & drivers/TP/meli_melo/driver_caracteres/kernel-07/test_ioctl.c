#include <sys/types.h>	/* open() */
#include <sys/stat.h>	/* open() */
#include <fcntl.h>	/* open() */
#include <sys/ioctl.h>	/* ioctl() */
#include <unistd.h>	/* close() */
#include <stdio.h>	/* sscanf() & perror() */
#include <stdlib.h>	/* atoi() */

int main (int argc, char **argv)
{
	int fd, ret;
	unsigned int arg;

	if (argc != 4) {
		printf("Usage: ioctl <device> <cmd (hexa)> <arg>\n");

		return 1;
	}

	ret = sscanf((char *)argv[2], "%x", &arg);
	if (ret <= 0) {
		printf("Usage:\nioctl <device> <cmd (hexa)> <arg>\n");
		printf("Command (0x%x) should be in hexadecimal\n",
			(unsigned int)argv[2]);
		perror("sscanf()");

		return 1;
	}

	fd = open((char *)argv[1], O_RDWR);
	if (fd < 0) {
		printf("Unable to open %s\n", argv[1]);
		perror("open()");

		return 1;
	}

	ret = ioctl(fd, (unsigned int)arg, atoi(argv[3]));
	if (ret < 0) {
		printf("Error doing \'ioctl %s 0x%x %d\'\n",
			argv[1], arg, atoi(argv[3]));
		perror("ioctl()");

		return 1;
	} else {
		printf("Command \'ioctl %s 0x%x %d\' ==> OK\n",
			argv[1], arg, atoi(argv[3]));
	}

	ret = close(fd);
	if (fd < 0) {
		printf("Unable to close file descriptor %d\n", fd);
		perror("close()");

		return 1;
	}

	return 0;
}
