#include "agent.h"

void free_agent_list(agent_list *h) {

	agent_list *n, *c = h;

	do {
		n = c->next;
		free(c);
		c = n;
	} while (c != NULL);
}
