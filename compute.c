#include "compute.h"

void *compute_shift(void *d) {

	shift_data *data = d;
	size_t i;

	for (i = 0; i < data->n; i++) {
		data->blocks[i] >>= data->l;
		if (i) data->blocks[i - 1] |= ((data->blocks[i]) & ((1 << data->l) - 1)) << (sizeof(row_block) * 8 - data->l);
	}

	free(data);
	pthread_exit(NULL);
}

void *compute_maximize(void *d) {

	max_data *data = d;
	row *row = malloc(sizeof(row));

	row->m = data->f->m;
	row->v = max(data->rows, data->i, data->j);
	row->blocks = malloc(data->f->m * (data->f->c / 8));
	memcpy(row->blocks, data->rows[data->i]->blocks, data->f->m * (data->f->c / 8));

	pthread_mutex_lock(data->m);
	data->f->rows[data->f->r] = row;
	data->f->r++;
	pthread_mutex_unlock(data->m);

	free(data);
	pthread_exit(NULL);
}

void *compute_joint_sum(void *d) {

	row *row;
	size_t i, j;
	sum_data *data = d;

	for (i = 0; i < data->f2->r; i++)
		if (compatible(data->f1, data->a, data->f2, i, data->sh)) {

			row = malloc(sizeof(row));
			row->m = data->f3->m;
			row->v = data->f1->rows[data->a]->v + data->f2->rows[i]->v;
			row->blocks = calloc(data->f3->m, data->f3->c / 8);
			memcpy(row->blocks, data->f1->rows[data->a]->blocks, data->f1->m * (data->f1->c / 8));

			for (j = 0; j < data->f2->n; j++)
				if (BIT(data->f2, i, j)) SETBIT(row->blocks, data->sh[j], data->f1->c);

			pthread_mutex_lock(data->m);
			data->f3->rows[data->f3->r] = row;
			data->f3->r++;
			pthread_mutex_unlock(data->m);
		}

	free(data);
	pthread_exit(NULL);
}
