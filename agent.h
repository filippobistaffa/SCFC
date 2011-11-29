#ifndef AGENT_H_
#define AGENT_H_

#include "scfc.h"

#define ECHO_AGENT 1
#define COLOR 1

struct agent {

	size_t id, n, l;
	agent *p, **pp;
	children **ch;
	function *luf, *pf;
	variable **vars;
	size_t *req;
	value payment;
};

struct children {

	agent *a;
	agent **pch;
};

struct agent_list {

	agent *agent;
	agent_list *next;
};

void free_agent_list(agent_list *h);
void compute_payment(agent *a);
void create_luf(agent *a);

#endif /* AGENT_H_ */
