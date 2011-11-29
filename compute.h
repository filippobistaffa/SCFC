#ifndef COMPUTE_H_
#define COMPUTE_H_

#include "scfc.h"

struct sum_data {

	function *f1, *f2, *f3;
	pthread_mutex_t *m;
	size_t a, *sh;
};

struct shift_data {

	row_block *blocks;
	size_t n, l;
};

struct max_data {

	pthread_mutex_t *m;
	function *f;
	size_t i, j;
	row **rows;
};


struct arg_data {

	pthread_mutex_t *m;
	row *row, *prow;
	row **max_row;
	size_t *sh;
};

void *compute_shift(void *d);
void *compute_arg_max(void *d);
void *compute_maximize(void *d);
void *compute_joint_sum(void *d);

#endif /* COMPUTE_H_ */
