#ifndef GRAPH_H_
#define GRAPH_H_

#include "scfc.h"
#include "list.h"
#include "agent.h"
#include "listsort.h"

#define BLOCK_NUMBER 1000
#define MAX_AGENTS 200
#define BLOCK 3
#define LINE 50

struct msg_data {

	pthread_mutex_t *mutex;
	pthread_cond_t *cond;
	agent *a;
};

struct tuple {

	variable *var;
	agent_list *agents;
};

void read_vars(char *filename, agent **agents);
agent **read_dot(char *filename, size_t *n);
agent_list *compute_pt(agent *a);

void *compute_dfs(void *d);
void *compute_vars(void *d);

#endif /* GRAPH_H_ */
