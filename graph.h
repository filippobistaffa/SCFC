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

struct dfs_data {

	pthread_mutex_t *mutex;
	pthread_cond_t *cond;
	agent *a;
};

void dfs(agent **agents, size_t n);
agent **read_dot(char *filename, size_t *n);
void read_vars(char *filename, agent **agents);

#endif /* GRAPH_H_ */
