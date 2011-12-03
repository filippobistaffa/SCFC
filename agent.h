#ifndef AGENT_H_
#define AGENT_H_

#include "scfc.h"

struct agent {

	size_t id;
	size_t n;
	size_t l;
	size_t d;
	size_t r;
	size_t ch_n; // # children
	size_t ch_id;

	function *luf, *pf;
	var_list *vars;
	size_t *req;

	tuple_list **req_msgs;

	row *assignment;
	value payment;

	agent_list *nv, *ngh, *pp, *pt, *token;
	agent *p, *sender;
	ch_list *ch;
};

struct child {

	agent *a;
	agent_list *pch;
};

char *assignment_to_string(agent *a);
char *variable_to_string(void *v);
char *agent_to_string(void *x);

void compute_payment(agent *a);
void create_luf(agent *a);

#endif /* AGENT_H_ */
