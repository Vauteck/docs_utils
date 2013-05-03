#include <unistd.h>

int main(int argc, char* argv[], char* env[])
{
	int rc = 0;
	
	write(STDOUT_FILENO, "Hello !\n", 8);

	return rc;
} /* main */

/* EOF */

