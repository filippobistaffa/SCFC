#ifndef GRAPH_H_
#define GRAPH_H_

#include "scfc.h"

#define BLOCK_NUMBER 1000
#define MAX_AGENTS 200
#define BLOCK 3
#define LINE 50

agent **read_dot(char *filename, size_t *n);
void read_vars(char *filename, agent **agents);

#endif /* GRAPH_H_ */
