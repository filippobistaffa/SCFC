#include "agent.h"

void free_agent_list(agent_list *h) {

	agent_list *n, *c = h;

	do {
		n = c->next;
		free(c);
		c = n;
	} while (c != NULL);
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
		printf("\033[1;36m[ A-%02zu ] Assignment\033[m\n", a->id);
#endif
	}

#if ALGORITHM_MESSAGES > 0
	printf("\033[1;32m[ A-%02zu ] Payment = %f\033[m\n", a->id, a->payment);
#endif
}
