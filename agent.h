#ifndef AGENT_H_
#define AGENT_H_

#include "scfc.h"

#define ECHO_AGENT 1
#define COLOR 1

struct agent {

	size_t id, n, l, d, r;

	function *luf, *pf;
	var_list *vars;
	size_t *req;

	row *assignment;
	value payment;

	agent_list *nv, *ngh, *pp, *token;
	agent *p, *sender;
	ch_list *ch;
};

struct child {

	agent *a;
	agent_list *pch;
};

struct tuple {

	variable *var;
	agent_list *agents;
};

char *assignment_to_string(agent *a);
char *variable_to_string(void *v);
char *agent_to_string(void *x);

void compute_payment(agent *a);
void create_luf(agent *a);

#endif /* AGENT_H_ */
