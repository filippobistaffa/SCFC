#include "threaded.h"

function *joint_sum(function *f1, function *f2) {

	function *sum = malloc(sizeof(function));

	sum->r = 0;

	/**
	 * Upper bound allocating, need to reallocate later
	 */

	size_t r = f1->n * f2->n;
	sum->n = f1->n + f2->n;
	sum->m = CEIL((float) sum->n / BLOCK_BITSIZE);
	sum->vars = calloc(sum->m, sizeof(agent **));
	sum->rows = malloc(r * sizeof(row *));

	size_t i, j, k, t;
	size_t s = f1->n, sh[f2->n];

	for (i = 0; i < f1->n; i++) {
		if (!sum->vars[i / BLOCK_BITSIZE]) sum->vars[i / BLOCK_BITSIZE] = malloc(BLOCK_BITSIZE * sizeof(agent *));
		VAR(sum, i) = VAR(f1, i);
	}

	for (j = 0; j < f2->n; j++) {
		for (i = 0; i < f1->n; i++)
			if (VAR(f1, i) == VAR(f2, j)) {
				sh[j] = i;
				goto loop;
			}

		VAR(sum, s) = VAR(f2, j);
		sh[j] = s++;
		loop: ;
	}

	sum->n = s;
	sum->m = CEIL((float) sum->n / BLOCK_BITSIZE);
	sum->vars = realloc(sum->vars, sum->m * sizeof(agent **));

	t = THREAD_NUMBER;
	j = 0;

	pthread_t threads[t];
	void *status;

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

#if THREAD_MESSAGES > 0
	printf("[ JSUM ] Using Blocks Of %zu Threads\n", t);
#endif

	sum_data *data[t];

	for (i = 0; i < f1->r; i++) {

		data[j] = malloc(sizeof(sum_data));
		data[j]->f1 = f1;
		data[j]->f2 = f2;
		data[j]->f3 = sum;
		data[j]->sh = sh;
		data[j]->a = i;
		data[j]->m = &mutex;
		j++;

		if (i + 1 == f1->r || j == t) {
			if (i + 1 > t) {
#if THREAD_MESSAGES > 0
				printf("[ JSUM ] Waiting %zu Threads\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], &status);
			}

#if THREAD_MESSAGES > 0
			printf("[ JSUM ] Starting %zu Threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_joint_sum, data[k]);

			if (i + 1 != f1->r) j = 0;
		}
	}

#if THREAD_MESSAGES > 0
	printf("[ JSUM ] Waiting %zu Final Threads\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], &status);

	pthread_mutex_destroy(&mutex);

	sum->rows = realloc(sum->rows, sum->r * sizeof(row *));

#if MEMORY_MESSAGES > 0
	printf("[MEMORY] Sum Function Dimension = %zu bytes\n", size(sum));
#endif

	return sum;
}

function *maximize(agent *a) {

#if ALGORITHM_MESSAGES > 0
	printf("\033[1;37m[ A-%02zu ] Creating Demand Message\033[m\n", a->id);
#endif

	function *max = malloc(sizeof(function));

	max->r = 0;
	max->n = a->pf->n - a->l;
	max->m = CEIL((float) max->n / BLOCK_BITSIZE);
	max->vars = calloc(max->m, sizeof(agent **));

	size_t i, j, k;

	for (i = 0; i < max->n; i++) {
		if (!max->vars[i / BLOCK_BITSIZE]) max->vars[i / BLOCK_BITSIZE] = malloc(BLOCK_BITSIZE * sizeof(agent *));
		VAR(max, i) = VAR(a->pf, i + a->l);
	}

	row **rows = malloc(a->pf->r * sizeof(row *));

	for (i = 0; i < a->pf->r; i++) {
		rows[i] = malloc(sizeof(row));
		rows[i]->m = a->pf->rows[i]->m;
		rows[i]->v = a->pf->rows[i]->v;
		rows[i]->blocks = malloc(rows[i]->m * sizeof(row_block));
		memcpy(rows[i]->blocks, a->pf->rows[i]->blocks, rows[i]->m * sizeof(row_block));
	}

	size_t t = THREAD_NUMBER;
	pthread_t threads[t];
	void *status;

#if THREAD_MESSAGES > 0
	printf("[ MAXI ] Using Blocks Of %zu Threads\n", t);
#endif

	shift_data *s_data[t];
	j = 0;

	for (i = 0; i < a->pf->r; i++) {

		s_data[j] = malloc(sizeof(shift_data));
		s_data[j]->blocks = rows[i]->blocks;
		s_data[j]->n = a->pf->n;
		s_data[j]->l = a->l;
		j++;

		if (i + 1 == a->pf->r || j == t) {
			if (i + 1 > t) {
#if THREAD_MESSAGES > 0
				printf("[RSHIFT] Waiting %zu Threads\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], &status);
			}

#if THREAD_MESSAGES > 0
			printf("[RSHIFT] Starting %zu Threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_shift, s_data[k]);

			if (i + 1 != a->pf->r) j = 0;
		}
	}

#if THREAD_MESSAGES > 0
	printf("[RSHIFT] Waiting %zu Final Threads\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], &status);

	qsort(rows, a->pf->r, sizeof(row *), compare_rows);

	/**
	 * Upper bound allocating, need to reallocate later
	 */

	size_t r = a->pf->r;
	max->rows = malloc(r * sizeof(row *));

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	max_data *m_data[t];
	size_t h = 0;
	j = 0;

	for (i = 0; i <= a->pf->r; i++) {

		if (!i || (i != a->pf->r && !compare_rows(&(rows[i]), &(rows[i - 1])))) continue;

		m_data[j] = malloc(sizeof(max_data));
		m_data[j]->rows = rows;
		m_data[j]->m = &mutex;
		m_data[j]->f = max;
		m_data[j]->i = h;
		m_data[j]->j = i;
		h = i;
		j++;

		if (i == a->pf->r || j == t) {
			if (i > t) {
#if THREAD_MESSAGES > 0
				printf("[ MAXI ] Waiting %zu Threads\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], &status);
			}

#if THREAD_MESSAGES > 0
			printf("[ MAXI ] Starting %zu Threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_maximize, m_data[k]);

			if (i != a->pf->r) j = 0;
		}
	}

#if THREAD_MESSAGES > 0
	printf("[ MAXI ] Waiting %zu Final Threads\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], &status);

	pthread_mutex_destroy(&mutex);
	max->rows = realloc(max->rows, max->r * sizeof(row *));

	for (i = 0; i < a->pf->r; i++) {
		free(rows[i]->blocks);
		free(rows[i]);
	}

	free(rows);

#if MEMORY_MESSAGES > 0
	printf("[MEMORY] Demand Message Dimension = %zu bytes\n", size(max));
#endif

	return max;
}

void get_arg_max(agent *a) {

	size_t i, j, k, t;
	size_t s = a->pf->n;
	size_t sh[a->p->pf->n];

	for (j = 0; j < a->p->pf->n; j++) {
		for (i = 0; i < a->pf->n; i++)
			if (VAR(a->pf, i) == VAR(a->p->pf, j)) {
				sh[j] = i;
				goto loop;
			}

		sh[j] = s++;
		loop: ;
	}

	t = THREAD_NUMBER;
	j = 0;

	a->assignment = NULL;
	pthread_t threads[t];
	void *status;

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

#if THREAD_MESSAGES > 0
	printf("[ARGMAX] Using Blocks Of %zu Threads\n", t);
#endif

	arg_data *data[t];

	for (i = 0; i < a->pf->r; i++) {

		data[j] = malloc(sizeof(arg_data));
		data[j]->max_row = &(a->assignment);
		data[j]->prow = a->p->assignment;
		data[j]->row = a->pf->rows[i];
		data[j]->m = &mutex;
		data[j]->sh = sh;
		j++;

		if (i + 1 == a->pf->r || j == t) {
			if (i + 1 > t) {
#if THREAD_MESSAGES > 0
				printf("[ARGMAX] Waiting %zu Threads\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], &status);
			}

#if THREAD_MESSAGES > 0
			printf("[ARGMAX] Starting %zu Threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_arg_max, data[k]);

			if (i + 1 != a->pf->r) j = 0;
		}
	}

#if THREAD_MESSAGES > 0
	printf("[ARGMAX] Waiting %zu Final Threads\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], &status);

#if ALGORITHM_MESSAGES > 0
	printf("\033[1;36m[ A-%02zu ] Assignment\033[m\n", a->id);
#endif

	pthread_mutex_destroy(&mutex);
}
