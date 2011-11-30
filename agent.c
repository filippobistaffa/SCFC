#include "agent.h"

char *variable_to_string(variable *v) {

	// XXX Works only for < 100 agents

	size_t i, j;
	const char post_mask[] = { 0xE2, 0x82, 0x80 };
	char *str = calloc(v->n * 7 + 3, 1);
	sprintf(str, "X");
	j = 1;

	for (i = 0; i < v->n; i++) {

		str[j++] = '.';

		if (v->agents[i]->id > 9) {
			memcpy(str + j, post_mask, 3);
			*(str + j + 2) |= v->agents[i]->id / 10;
			j += 3;
		}

		memcpy(str + j, post_mask, 3);
		*(str + j + 2) |= v->agents[i]->id % 10;
		j += 3;
	}

	return str;
}

char *assignment_to_string(agent *a) {

	char *var, *str = malloc(MAX_STRING_SIZE);
	size_t i, l, j = 0;
	str[j++] = '{';

	for (i = 0; i < a->l; i++)
		if (GETBIT(a->assignment, i)) {
			var = variable_to_string(VAR(a->pf, i));
			l = strlen(var);
			memcpy(str + j, var, l);
			strcpy(str + j + l, " ,");
			j += (l + 2);
			free(var);
		}

	strcpy(str + j - 2, "}");
	return realloc(str, strlen(str) + 1);
}

void create_luf(agent *a) {

#if ALGORITHM_MESSAGES > 0
	printf("\033[1;37m[ A-%02zu ] Creating LUF Function\033[m\n", a->id);
#endif

	size_t i;
	function *luf = malloc(sizeof(function));

	luf->id = a->id;
	luf->r = luf->n = a->n;
	luf->m = CEIL((float) luf->n / BLOCK_BITSIZE);
	luf->vars = malloc(luf->m * sizeof(agent **));
	luf->rows = malloc(luf->r * sizeof(row *));

	for (i = 0; i < luf->m; i++)
		luf->vars[i] = malloc(BLOCK_BITSIZE * sizeof(agent *));

	for (i = 0; i < a->n; i++)
		VAR(luf, i) = a->vars[i];

	for (i = 0; i < luf->r; i++) {
		luf->rows[i] = malloc(sizeof(row));
		luf->rows[i]->blocks = calloc(luf->m, sizeof(row_block));
		luf->rows[i]->m = luf->m;
		luf->rows[i]->n = luf->n;
	}

	for (i = 0; i < luf->n; i++) {
		SETBIT(luf->rows[i], i);
		if (i < a->l)
			luf->rows[i]->v = a->vars[i]->worth;
		else
			SETBIT(luf->rows[i], a->req[i - a->l]);
	}

#if MEMORY_MESSAGES > 0
	printf("[MEMORY] LUF Function Dimension = %zu Bytes\n", size(luf));
#endif

	a->luf = luf;
}

void compute_payment(agent *a) {

	if (a->p)
		a->payment = max(a->pf->rows, 0, a->pf->r)->v;
	else {
		a->assignment = max(a->pf->rows, 0, a->pf->r);
		a->payment = a->assignment->v;

#if ALGORITHM_MESSAGES > 0
		char *str = assignment_to_string(a);
		printf("\033[1;36m[ A-%02zu ] Active Local Variables = %s\033[m\n", a->id, str);
		free(str);
#endif
	}

#if ALGORITHM_MESSAGES > 0
	printf("\033[1;32m[ A-%02zu ] Payment = %f\033[m\n", a->id, a->payment);
#endif
}
