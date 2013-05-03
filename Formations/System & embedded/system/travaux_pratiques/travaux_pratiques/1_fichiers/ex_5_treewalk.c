/*
 * Exemple d'utilisation de la fonction ftw
 * Liste récursivement les contenus depuis le répertoire donné en argument
 * au programme
 */

#include <ftw.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

static int ftw_cb(const char *fpath, const struct stat *sb, int flag)
{
	printf("%-3s %7jd %-40s\n",
			(flag == FTW_F) ?   "f"   :			// regular file
			(flag == FTW_D) ?   "d"   :			// directory
			(flag == FTW_DNR) ? "dnr" :			// read only directory
			(flag == FTW_NS) ?  "ns"  :			// stat() failed
			"???",
			(intmax_t) sb->st_size,
			fpath);
	return 0;
}

int main(int argc, char **argv)
{
	/* If directory is not given as argument, use the current one */
	if (ftw((argc < 2) ? "." : argv[1], ftw_cb, 20) == -1) {
		perror("ftw");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
