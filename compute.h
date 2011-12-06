#ifndef COMPUTE_H_
#define COMPUTE_H_

#include "scfc.h"

struct sum_data {

	function *f1, *f2, *f3;
	pthread_mutex_t *m;
	size_t a, b, *sh;
};

struct shift_data {

	size_t a, b, l;
	row **rows;
};

struct max_data {

	pthread_mutex_t *m;
	function *f;
	size_t i, j;
	row **rows;
};

struct arg_data {

	row **max_row, **rows, *prow;
	pthread_mutex_t *m;
	size_t a, b, *sh;
};

void *compute_shift(void *d);
void *compute_arg_max(void *d);
void *compute_maximize(void *d);
void *compute_joint_sum(void *d);

#endif /* COMPUTE_H_ */
