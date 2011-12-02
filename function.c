#include "function.h"

int compatible(row *r1, row *r2, size_t *sh) {

	if (!sh) return 1;
	size_t i;

	for (i = 0; i < r2->n; i++)
		if ((sh[i] < r1->n) && (GETBIT(r1, sh[i]) ^ GETBIT(r2, i))) return 0;

	return 1;
}

row *max(row **rows, size_t i, size_t j) {

	row *max = rows[i];

	for (; i < j; i++)
		if (rows[i]->v > max->v) max = rows[i];

	return max;
}

int compare_rows(const void *a, const void *b) {

	const row *ra = *((row **) a);
	const row *rb = *((row **) b);

	size_t i;

	for (i = 0; i < ra->m; i++)
		if (ra->blocks[i] != rb->blocks[i]) return (int) ra->blocks[i] - (int) rb->blocks[i];

	return 0;
}

void subtract(function *f, value v) {

	size_t i;

	for (i = 0; i < f->r; i++)
		f->rows[i]->v -= v;
}

void nuke(function *f) {

#if MEMORY_MESSAGES > 0
	printf("[MEMORY] Free %zu Bytes\n", size(f));
#endif

	size_t i;

	free_list(LIST(f->vars));

	for (i = 0; i < f->r; i++) {
		free(f->rows[i]->blocks);
		free(f->rows[i]);
	}

	free(f->rows);
}

size_t size(function *f) {

	return sizeof(function) + f->r * (sizeof(row *) + sizeof(row) + f->m * sizeof(row_block)) + f->m * f->n * sizeof(agent *);
}
