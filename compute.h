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

	row **rows;
	function *f;
	pthread_mutex_t *m;
	size_t i, j;
};


struct arg_data {

	row **row;
	value *max;
	pthread_mutex_t *m;
};

void *compute_shift(void *d);
void *compute_arg_max(void *d);
void *compute_maximize(void *d);
void *compute_joint_sum(void *d);

#endif /* COMPUTE_H_ */
