/*
 * Utilise SIGCHLD pour attendre la terminaison d'un processus fils.
 */

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile sig_atomic_t child_not_exited = 1;

void clean_up_child(int sig __attribute__ ((unused)))
{
	int status;
	wait(&status);
	child_not_exited = 0;
}

int main(void)
{
	struct sigaction act;
	pid_t child;

	/* Assigne la fonction clean_up_child() au signal SIGCHLD */
	memset(&act, 0, sizeof(act));
	act.sa_handler = &clean_up_child;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGCHLD, &act, 0) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	/* Créé un processus fils */
	if ((child = fork()) == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (child == 0) {
		printf("in child\n");
		sleep(3);
		exit(EXIT_SUCCESS);
	}

	printf("in parent\n");
	while (child_not_exited) {
		printf("parent waiting...\n");
		sleep(1);
	}
	printf("child exited\n");

	exit(EXIT_SUCCESS);
}
