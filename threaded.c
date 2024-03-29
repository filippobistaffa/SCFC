#include "threaded.h"

size_t *shared(function *f1, function *f2, size_t *n, var_list *s) {

	size_t i, j, *sh = malloc(f2->n * sizeof(size_t));
	var_list *h, *k = f2->vars;
	*n = f1->n;
	j = 0;

	while (k) {
		h = f1->vars;
		i = 0;
		while (h) {
			if (h->v == k->v) {
				sh[j] = i;
				goto loop;
			}
			h = h->n;
			i++;
		}
		if (s) add(LIST(s), k->v);
		sh[j] = (*n)++;
		loop: k = k->n;
		j++;
	}

	return sh;
}

function *joint_sum(function *f1, function *f2) {

	function *sum = malloc(sizeof(function));
	sum->rows = malloc(LIMIT_MEMORY(f1->r * f2->r * sizeof(row *)));
	sum->vars = VAR_LIST(copy_list(LIST(f1->vars)));
	sum->r = 0;
	size_t *sh = shared(f1, f2, &(sum->n), sum->vars);
	sum->m = CEIL((float) sum->n / BLOCK_BITSIZE);
	size_t i, k, t;

	if (f1->r > THREAD_NUMBER) {
		t = THREAD_NUMBER;
		k = f1->r / t;
	} else {
		t = f1->r;
		k = 1;
	}

#if THREAD_MESSAGES > 0
	printf("[JSUM] Using Blocks Of %zu Threads\n", t);
#endif

	sum_data *data[t];
	pthread_t threads[t];
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	for (i = 0; i < t; i++) {

		data[i] = malloc(sizeof(sum_data));
		data[i]->f1 = f1;
		data[i]->f2 = f2;
		data[i]->f3 = sum;
		data[i]->sh = sh;
		data[i]->a = i * k;
		data[i]->b = (i == t - 1) ? (f1->r - 1) : ((i + 1) * k - 1);
		data[i]->m = &mutex;

		pthread_create(&threads[i], NULL, compute_joint_sum, data[i]);
	}

	for (i = 0; i < t; i++)
		pthread_join(threads[i], NULL);

	pthread_mutex_destroy(&mutex);
	sum->rows = realloc(sum->rows, sum->r * sizeof(row *));

#if MEMORY_MESSAGES > 0
	printf("[MEMO] Sum Function Dimension = %zu bytes\n", size(sum));
#endif

	free(sh);
	return sum;
}

function *maximize(agent *a) {

#if ALGORITHM_MESSAGES > 0
	printf("\033[1;37m[A-%02zu] Creating Demand Message\033[m\n", a->id);
#endif

	size_t i, k, t;
	row **rows = malloc(a->pf->r * sizeof(row *));

	for (i = 0; i < a->pf->r; i++) {
		rows[i] = malloc(sizeof(row));
		rows[i]->m = a->pf->rows[i]->m;
		rows[i]->v = a->pf->rows[i]->v;
		rows[i]->blocks = malloc(rows[i]->m * sizeof(row_block));
		memcpy(rows[i]->blocks, a->pf->rows[i]->blocks, rows[i]->m * sizeof(row_block));
	}

	if (a->pf->r > THREAD_NUMBER) {
		t = THREAD_NUMBER;
		k = a->pf->r / t;
	} else {
		t = a->pf->r;
		k = 1;
	}

	pthread_t threads[t];
	shift_data *s_data[t];

#if THREAD_MESSAGES > 0
	printf("[R-SH] Using Blocks Of %zu Threads\n", t);
#endif

	for (i = 0; i < t; i++) {

		s_data[i] = malloc(sizeof(shift_data));
		s_data[i]->rows = rows;
		s_data[i]->l = a->l;
		s_data[i]->a = i * k;
		s_data[i]->b = (i == t - 1) ? (a->pf->r - 1) : ((i + 1) * k - 1);

		pthread_create(&threads[i], NULL, compute_shift, s_data[i]);
	}

	for (i = 0; i < t; i++)
		pthread_join(threads[i], NULL);

	function *max = malloc(sizeof(function));

	max->r = 0;
	max->n = a->pf->n - a->l;
	max->m = CEIL((float) max->n / BLOCK_BITSIZE);
	max->vars = VAR_LIST(remove_first(copy_list(LIST(a->pf->vars)), a->l));
	max->rows = malloc(a->pf->r * sizeof(row *));

	for (i = 0; i < a->pf->r; i++)
		rows[i]->m = max->m;

	qsort(rows, a->pf->r, sizeof(row *), compare_rows);

	for (i = 0; i < a->pf->r; i++) {
		if (!i || compare_rows(&(rows[i]), &(rows[i - 1]))) {
			row *r = max->rows[max->r++] = malloc(sizeof(row));
			r->n = max->n;
			r->m = max->m;
			r->v = rows[i]->v;
			r->blocks = malloc(r->m * sizeof(row_block));
			memcpy(r->blocks, rows[i]->blocks, r->m * sizeof(row_block));
		} else if (rows[i]->v > max->rows[max->r - 1]->v) max->rows[max->r - 1]->v = rows[i]->v;
	}

	max->rows = realloc(max->rows, max->r * sizeof(row *));

	for (i = 0; i < a->pf->r; i++) {
		free(rows[i]->blocks);
		free(rows[i]);
	}

	free(rows);
	rows = malloc(max->r * sizeof(row *));

	cmph_io_adapter_t *source = cmph_io_custom_vector_adapter(max->rows, max->r);
	cmph_config_t *config = cmph_config_new(source);
	cmph_config_set_algo(config, CMPH_CHD);
	cmph_t *hash = cmph_new(config);
	cmph_io_vector_adapter_destroy(source);

	for (i = 0; i < max->r; i++)
		rows[cmph_search(hash, (const char *) max->rows[i]->blocks, max->m * sizeof(row_block))] = max->rows[i];

	free(max->rows);
	max->rows = rows;

#if MEMORY_MESSAGES > 0
	printf("[MEMO] Demand Message Dimension = %zu bytes\n", size(max));
#endif

	return max;
}

void get_arg_max(agent *a) {

	size_t i, k, s, t;
	size_t *sh = shared(a->pf, a->p->pf, &s, NULL);

	if (a->pf->r > THREAD_NUMBER) {
		t = THREAD_NUMBER;
		k = a->pf->r / t;
	} else {
		t = a->pf->r;
		k = 1;
	}

#if THREAD_MESSAGES > 0
	printf("[AMAX] Using Blocks Of %zu Threads\n", t);
#endif

	arg_data *data[t];
	pthread_t threads[t];
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	for (i = 0; i < t; i++) {

		data[i] = malloc(sizeof(arg_data));
		data[i]->max_row = &(a->assignment);
		data[i]->prow = a->p->assignment;
		data[i]->rows = a->pf->rows;
		data[i]->sh = sh;
		data[i]->a = i * k;
		data[i]->m = &mutex;
		data[i]->b = (i == t - 1) ? (a->pf->r - 1) : ((i + 1) * k - 1);

		pthread_create(&threads[i], NULL, compute_arg_max, data[i]);
	}

	for (i = 0; i < t; i++)
		pthread_join(threads[i], NULL);

	active_local_variable(a);

#if ALGORITHM_MESSAGES > 0
	char *str = variable_to_string(a->v);
	printf("\033[1;36m[A-%02zu] Active Local Variable = %s\033[m\n", a->id, str);
	free(str);
#endif

	free(sh);
	pthread_mutex_destroy(&mutex);
}
