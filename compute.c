#include "compute.h"

void *compute_left_shift(void *d) {

	shift_data *data = d;
	size_t i, s = data->n;
	unsigned char *byte, bit;
	void *blocks = data->block;

	for (i = 0; i < data->l; i++)
		for (byte = blocks; s--; ++byte) {
			bit = 0;
			if (s) bit = byte[1] & (1 << (CHAR_BIT - 1)) ? 1 : 0;
			*byte <<= 1;
			*byte |= bit;
		}

	free(data);
	pthread_exit(NULL);
}

void *compute_maximize(void *d) {

	max_data *data = d;
	row *row = malloc(sizeof(row));

	row->m = data->f1->m;
	row->v = max(data->f1, data->i, data->j);
	row->blocks = calloc(data->f2->m, data->f2->c / 8);
	memcpy(row->blocks, data->blocks, data->f1->m * (data->f1->c / 8));

	pthread_mutex_lock(data->m);
	data->f2->rows[data->f2->r] = row;
	data->f2->r++;
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
