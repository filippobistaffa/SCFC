#ifndef AGENT_H_
#define AGENT_H_

#include "scfc.h"

struct agent {

	size_t id;
	size_t n; // # vars
	size_t l; // # local vars
	size_t d; // degree
	size_t r; // is root
	size_t ch_n; // # children
	size_t ch_id;
	size_t level;

	variable *v;

	function *luf, *pf;
	var_list *vars;
	size_t *req;

	function **dem_msgs;
	tuple_list **req_msgs;

	row *assignment;
	value payment, single;

	agent_list *nv, *ngh, *pp, *pt, *token;
	agent *p, *sender;
	ch_list *ch;
};

struct child {

	agent *a;
	agent_list *pch;
};

void active_local_variable(agent *a);
char *variable_to_string(void *v);
char *agent_to_string(void *x);

void compute_payment(agent *a, pthread_cond_t *cond, pthread_mutex_t *mutex);
void compute_luf(agent *a, value **data, size_t users, size_t days, value (*worth)(variable *, value **, size_t, size_t));

#endif /* AGENT_H_ */
