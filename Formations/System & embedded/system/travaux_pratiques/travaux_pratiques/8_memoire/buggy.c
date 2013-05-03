/*
 * buggy program
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct linked_list {
	struct linked_list *next;
	char *data;
};

int main(void)
{
	struct linked_list *head = NULL;
	struct linked_list *temp = NULL;
	char *test_str = "Linux rulez";

	for (int ii = 0; ii < 10; ii++) {
		temp = malloc(sizeof(*temp));
		/* Bug : temp->data isn't allocated */
		strncpy(temp->data, test_str, strlen(test_str));
		temp->next = head;
		head = temp;
	}

	/* Bad : memory not freed */

	exit(EXIT_SUCCESS);
}

