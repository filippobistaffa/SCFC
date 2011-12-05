#include "threaded.h"

size_t *shared(function *f1, function *f2, size_t *n, var_list *s) {

	size_t i, j, *sh = calloc(1, f2->n * sizeof(size_t));
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
	sum->rows = malloc(f1->n * f2->n * sizeof(row *));
	sum->vars = VAR_LIST(copy_list(LIST(f1->vars)));
	sum->r = 0;
	size_t *sh = shared(f1, f2, &(sum->n), sum->vars);
	sum->m = CEIL((float) sum->n / BLOCK_BITSIZE);
	size_t i, j, k, t, w;

	t = f1->r < THREAD_NUMBER ? f1->r : THREAD_NUMBER;
printf("%zu\n", t);

	t = THREAD_NUMBER;
	j = 0, w = 0;
	t = 1;

	pthread_t threads[t];
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

			if (w) {
#if THREAD_MESSAGES > 0
				printf("[ JSUM ] Waiting %zu Threads\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], NULL);
			}

#if THREAD_MESSAGES > 0
			printf("[ JSUM ] Starting %zu Threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_joint_sum, data[k]);

			w = 1;
			if (i + 1 != f1->r) j = 0;
		}
	}

#if THREAD_MESSAGES > 0
	printf("[ JSUM ] Waiting %zu Final Threads\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], NULL);

	pthread_mutex_destroy(&mutex);
	sum->rows = realloc(sum->rows, sum->r * sizeof(row *));

#if MEMORY_MESSAGES > 0
	printf("[MEMORY] Sum Function Dimension = %zu bytes\n", size(sum));
#endif

	free(sh);
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
	max->vars = VAR_LIST(remove_first(copy_list(LIST(a->pf->vars)), a->l));

	size_t i, j, k, w;
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

#if THREAD_MESSAGES > 0
	printf("[ MAXI ] Using Blocks Of %zu Threads\n", t);
#endif

	shift_data *s_data[t];
	j = 0, w = 0;

	for (i = 0; i < a->pf->r; i++) {

		s_data[j] = malloc(sizeof(shift_data));
		s_data[j]->blocks = rows[i]->blocks;
		s_data[j]->m = a->pf->m;
		s_data[j]->l = a->l;
		j++;

		if (i + 1 == a->pf->r || j == t) {

			if (w) {
#if THREAD_MESSAGES > 0
				printf("[RSHIFT] Waiting %zu Threads\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], NULL);
			}

#if THREAD_MESSAGES > 0
			printf("[RSHIFT] Starting %zu Threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_shift, s_data[k]);

			w = 1;
			if (i + 1 != a->pf->r) j = 0;
		}
	}

#if THREAD_MESSAGES > 0
	printf("[RSHIFT] Waiting %zu Final Threads\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], NULL);

	qsort(rows, a->pf->r, sizeof(row *), compare_rows);
	size_t h, r = a->pf->r;
	max->rows = malloc(r * sizeof(row *));

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	max_data *m_data[t];
	w = 0, h = 0, j = 0;

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

			if (w) {
#if THREAD_MESSAGES > 0
				printf("[ MAXI ] Waiting %zu Threads\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], NULL);
			}

#if THREAD_MESSAGES > 0
			printf("[ MAXI ] Starting %zu Threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_maximize, m_data[k]);

			w = 1;
			if (i != a->pf->r) j = 0;
		}
	}

#if THREAD_MESSAGES > 0
	printf("[ MAXI ] Waiting %zu Final Threads\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], NULL);

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

	size_t i, j, k, s, t, w;
	size_t *sh = shared(a->pf, a->p->pf, &s, NULL);

	t = THREAD_NUMBER;
	j = 0, w = 0;

	pthread_t threads[t];
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
			if (w) {
#if THREAD_MESSAGES > 0
				printf("[ARGMAX] Waiting %zu Threads\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], NULL);
			}

#if THREAD_MESSAGES > 0
			printf("[ARGMAX] Starting %zu Threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_arg_max, data[k]);

			w = 1;
			if (i + 1 != a->pf->r) j = 0;
		}
	}

#if THREAD_MESSAGES > 0
	printf("[ARGMAX] Waiting %zu Final Threads\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], NULL);

#if ALGORITHM_MESSAGES > 0
	char *str = assignment_to_string(a);
	printf("\033[1;36m[ A-%02zu ] Active Local Variables = %s\033[m\n", a->id, str);
	free(str);
#endif

	pthread_mutex_destroy(&mutex);
}
