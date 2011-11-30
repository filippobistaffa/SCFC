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
	row *assignment;
};

struct children {

	agent *a;
	agent **pch;
};

char *assignment_to_string(agent *a);
void compute_payment(agent *a);
void create_luf(agent *a);

#endif /* AGENT_H_ */
