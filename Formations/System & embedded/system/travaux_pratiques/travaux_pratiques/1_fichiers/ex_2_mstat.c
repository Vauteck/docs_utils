/*
 * Exemple d'utilisation de la fonction lstat
 * Le nom de fichier est donné en argument au programme
 */

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

int main(int argc, char **argv)
{
	struct stat buf;
	mode_t mode;
	char type[80];

	if (argc != 2) {
		fprintf(stderr, "USAGE: %s <filename>\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (0 > (lstat(argv[1], &buf))) {
		fprintf(stderr, "Errno = %d\n", errno);
		perror("lstat");
		return EXIT_FAILURE;
	}

	printf("File: %s\n", argv[1]);
	mode = buf.st_mode;
	if (S_ISLNK(mode))
		strcpy(type, "symbolic link");
	else if (S_ISREG(mode))
		strcpy(type, "regular file");
	else if (S_ISDIR(mode))
		strcpy(type, "directory");
	else if (S_ISCHR(mode))
		strcpy(type, "character device");
	else if (S_ISBLK(mode))
		strcpy(type, "block device");
	else if (S_ISFIFO(mode))
		strcpy(type, "FIFO");
	else if (S_ISSOCK(mode))
		strcpy(type, "socket");
	else strcpy(type, "unknown type");
	printf("Type: %s\n", type);
	printf("Inode: %d\n", (int) buf.st_ino);
	printf("Device: %d,%d\n", major(buf.st_dev), minor(buf.st_dev));
	printf("Mode: %#o\n", (int) buf.st_mode & ~(S_IFMT));
	printf("Links: %d\n", (int) buf.st_nlink);
	printf("UID: %d\n", buf.st_uid);
	printf("GID: %d\n", buf.st_gid);
	printf("Size: %ld\n", (long int) buf.st_size);
	printf("Block size: %d\n", (int) buf.st_blksize);
	printf("Blocks: %d\n", (int) buf.st_blocks);
	printf("Accessed: %s", ctime(&buf.st_atime));
	printf("Modified: %s", ctime(&buf.st_mtime));
	printf("Changed: %s", ctime(&buf.st_ctime));

	return EXIT_SUCCESS;
}
