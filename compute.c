#include "compute.h"

void *compute_shift(void *d) {

	shift_data *data = d;
	size_t i, j, k = data->l / (sizeof(row_block) * 8);

	for (i = data->a; i <= data->b; i++) {

		if (k) {
			for (j = 0; j < data->rows[i]->m; j++)
				if (j + k >= data->rows[i]->m)
					data->rows[i]->blocks[j] = 0;
				else
					data->rows[i]->blocks[j] = data->rows[i]->blocks[j + k];
			data->l -= k * 8 * sizeof(row_block);
		}

		for (j = 0; j < data->rows[i]->m; j++) {
			if (j) data->rows[i]->blocks[j - 1] |= ((data->rows[i]->blocks[j]) & ((1 << data->l) - 1)) << (sizeof(row_block) * 8 - data->l);
			data->rows[i]->blocks[j] >>= data->l;
		}
	}

	free(data);
	pthread_exit(NULL);
}

void *compute_maximize(void *d) {

	max_data *data = d;
	row *r = malloc(sizeof(row));

	r->n = data->f->n;
	r->m = data->f->m;
	r->v = max(data->rows, data->i, data->j)->v;
	r->blocks = malloc(data->f->m * sizeof(row_block));
	memcpy(r->blocks, data->rows[data->i]->blocks, data->f->m * sizeof(row_block));

	pthread_mutex_lock(data->m);
	data->f->rows[data->f->r] = r;
	data->f->r++;
	pthread_mutex_unlock(data->m);

	free(data);
	pthread_exit(NULL);
}

void *compute_joint_sum(void *d) {

	row *r;
	size_t i, j, k;
	sum_data *data = d;

	for (i = data->a; i <= data->b; i++)
		for (j = 0; j < data->f2->r; j++)
			if (compatible(data->f1->rows[i], data->f2->rows[j], data->sh)) {

				r = malloc(sizeof(row));
				r->n = data->f3->n;
				r->m = data->f3->m;
				r->v = data->f1->rows[i]->v + data->f2->rows[j]->v;
				r->blocks = calloc(data->f3->m, sizeof(row_block));
				memcpy(r->blocks, data->f1->rows[i]->blocks, data->f1->m * sizeof(row_block));

				for (k = 0; k < data->f2->n; k++)
					if (GETBIT(data->f2->rows[j], k)) SETBIT(r, data->sh[k]);

				pthread_mutex_lock(data->m);
				data->f3->rows[data->f3->r] = r;
				data->f3->r++;
				pthread_mutex_unlock(data->m);
			}

	free(data);
	pthread_exit(NULL);
}

void *compute_arg_max(void *d) {

	arg_data *data = d;
	size_t i;

	for (i = data->a; i <= data->b; i++)
		if (compatible(data->rows[i], data->prow, data->sh)) {
			pthread_mutex_lock(data->m);
			if (!*(data->max_row) || data->rows[i]->v > (*(data->max_row))->v) *(data->max_row) = data->rows[i];
			pthread_mutex_unlock(data->m);
		}

	free(data);
	pthread_exit(NULL);
}
