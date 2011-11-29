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

	for (i = l; i < f->n; i++) {
		if (!max->vars[i / max->c]) max->vars[i / max->c] = malloc(max->c * sizeof(agent *));
		VAR(max, i - l) = VAR(f, i);
	}

	size_t t = sysconf(_SC_NPROCESSORS_CONF) * THREADS_PER_CORE;
	pthread_t threads[t];
	void *status;

#if ECHO > 0
	printf("[Maximize] Using blocks of %zu threads\n", t);
#endif

	shift_data *shift_data[t];

	for (i = 0; i < f->r; i++) {

		shift_data[j] = malloc(sizeof(sum_data));
		shift_data[j]->block = f->rows[i]->blocks;
		shift_data[j]->n = f->n;
		shift_data[j]->l = l;
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
				pthread_create(&threads[k], NULL, compute_left_shift, shift_data[k]);

			if (i + 1 != f->r) j = 0;
		}
	}

#if ECHO > 0
	printf("[Left Shift] Waiting for the final %zu threads to finish\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], &status);

	qsort(f->rows, f->r, sizeof(row *), compare_rows);

	/**
	 * Upper bound allocating, need to reallocate later
	 */

	size_t r = f->r;
	max->rows = malloc(r * sizeof(row *));

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	max_data *max_data[t];
	size_t h = 0;

	for (i = 0; i < f->r; i++) {

		if (!i || (i + 1 != f->r && !compare_rows(&(f->rows[i]->blocks), &(f->rows[i - 1]->blocks)))) continue;

		max_data[j] = malloc(sizeof(max_data));
		max_data[j]->blocks = f->rows[i]->blocks;
		max_data[j]->m = &mutex;
		max_data[j]->f2 = max;
		max_data[j]->f1 = f;
		max_data[j]->i = h;
		j++;

		if (i + 1 == f->r || j == t) {
			if (i + 1 > t) {
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
				pthread_create(&threads[k], NULL, compute_left_shift, max_data[k]);

			if (i + 1 != f->r) j = 0;
		}
	}

#if ECHO > 0
	printf("[Maximize] Waiting for the final %zu threads to finish\n", j);
#endif

	for (k = 0; k < j; k++)
		pthread_join(threads[k], &status);

	pthread_mutex_destroy(&mutex);

	nuke(f);
	max->rows = realloc(max->rows, max->r * sizeof(row *));
	return max;
}

