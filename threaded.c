#include "threaded.h"

function *joint_sum(function *f1, function *f2) {

	function *sum = malloc(sizeof(function));

	sum->r = 0;
	sum->c = f1->c;

	/**
	 * Upper bound allocating, need to reallocate later
	 */

	size_t r = f1->n * f2->n;
	sum->n = f1->n + f2->n;
	sum->m = CEIL((float) sum->n / sum->c);
	sum->vars = calloc(sum->m, sizeof(agent **));
	sum->rows = malloc(r * sizeof(row *));

	size_t i, j, k, t;
	size_t s = f1->n, sh[f2->n];

	for (i = 0; i < f1->n; i++) {
		if (!sum->vars[i / sum->c]) sum->vars[i / sum->c] = malloc(sum->c * sizeof(agent *));
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
	sum->m = CEIL((float) sum->n / sum->c);
	sum->vars = realloc(sum->vars, sum->m * sizeof(agent **));

	t = sysconf(_SC_NPROCESSORS_CONF) * THREADS_PER_CORE;
	j = 0;

	pthread_t threads[t];
	void *status;

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

#if ECHO > 0
	printf("[Sum] Using blocks of %zu threads\n", t);
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
#if ECHO > 0
				printf("[Sum] Waiting for %zu threads to finish\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], &status);
			}

#if ECHO > 0
			printf("[Sum] Starting %zu new threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_joint_sum, data[k]);

			if (i + 1 != f1->r) j = 0;
		}
	}

#if ECHO > 0
	printf("[Sum] Waiting for the final %zu threads to finish\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], &status);

	pthread_mutex_destroy(&mutex);

	sum->rows = realloc(sum->rows, sum->r * sizeof(row *));
	return sum;
}

function *maximize(function *f, size_t l) {

	function *max = malloc(sizeof(function));

	max->r = 0;
	max->c = f->c;
	max->n = f->n - l;
	max->m = CEIL((float) max->n / max->c);
	max->vars = calloc(max->m, sizeof(agent **));

	size_t i, j, k;

	for (i = 0; i < max->n; i++) {
		if (!max->vars[i / max->c]) max->vars[i / max->c] = malloc(max->c * sizeof(agent *));
		VAR(max, i) = VAR(f, i + l);
	}

	row **rows = malloc(f->r * sizeof(row *));

	for (i = 0; i < f->r; i++) {
		rows[i] = malloc(sizeof(row));
		rows[i]->m = f->rows[i]->m;
		rows[i]->v = f->rows[i]->v;
		rows[i]->blocks = malloc(rows[i]->m * sizeof(row_block));
		memcpy(rows[i]->blocks, f->rows[i]->blocks, rows[i]->m * sizeof(row_block));
	}

	size_t t = sysconf(_SC_NPROCESSORS_CONF) * THREADS_PER_CORE;
	pthread_t threads[t];
	void *status;

	t = 1;

#if ECHO > 0
	printf("[Maximize] Using blocks of %zu threads\n", t);
#endif

	shift_data *s_data[t];
	j = 0;

	for (i = 0; i < f->r; i++) {

		s_data[j] = malloc(sizeof(shift_data));
		s_data[j]->blocks = rows[i]->blocks;
		s_data[j]->n = f->n;
		s_data[j]->l = l;
		j++;

		if (i + 1 == f->r || j == t) {
			if (i + 1 > t) {
#if ECHO > 0
				printf("[Left Shift] Waiting for %zu threads to finish\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], &status);
			}

#if ECHO > 0
			printf("[Left Shift] Starting %zu new threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_shift, s_data[k]);

			if (i + 1 != f->r) j = 0;
		}
	}

#if ECHO > 0
	printf("[Left Shift] Waiting for the final %zu threads to finish\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], &status);

	qsort(rows, f->r, sizeof(row *), compare_rows);

	/**
	 * Upper bound allocating, need to reallocate later
	 */

	size_t r = f->r;
	max->rows = malloc(r * sizeof(row *));

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	max_data *m_data[t];
	j = 0;
	size_t h = 0;

	for (i = 0; i <= f->r; i++) {

		if (!i || (i != f->r && !compare_rows(&(rows[i]), &(rows[i - 1])))) continue;

		m_data[j] = malloc(sizeof(max_data));
		m_data[j]->blocks = rows[i - 1]->blocks;
		m_data[j]->m = &mutex;
		m_data[j]->f2 = max;
		m_data[j]->f1 = f;
		m_data[j]->i = h;
		m_data[j]->j = i;
		h = i;
		j++;

		if (i == f->r || j == t) {
			if (i > t) {
#if ECHO > 0
				printf("[Maximize] Waiting for %zu threads to finish\n", t);
#endif
				for (k = 0; k < t; k++)
					pthread_join(threads[k], &status);
			}

#if ECHO > 0
			printf("[Maximize] Starting %zu new threads\n", j);
#endif
			for (k = 0; k < j; k++)
				pthread_create(&threads[k], NULL, compute_maximize, m_data[k]);

			if (i != f->r) j = 0;
		}
	}

#if ECHO > 0
	printf("[Maximize] Waiting for the final %zu threads to finish\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], &status);

	pthread_mutex_destroy(&mutex);
	max->rows = realloc(max->rows, max->r * sizeof(row *));

	for (i = 0; i < f->r; i++) {
		free(rows[i]->blocks);
		free(rows[i]);
	}

	free(rows);
	return max;
}
