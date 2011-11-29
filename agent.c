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

	size_t i;
	function *luf = malloc(sizeof(function));

	luf->id = a->id;
	luf->r = luf->n = a->n;
	luf->c = sizeof(row_block) * 8;
	luf->m = CEIL((float) luf->n / luf->c);
	luf->vars = malloc(luf->m * sizeof(agent **));
	luf->rows = malloc(luf->r * sizeof(row *));

	for (i = 0; i < luf->m; i++)
		luf->vars[i] = malloc(luf->c * sizeof(agent *));

	for (i = 0; i < a->n; i++)
		VAR(luf, i) = a->vars[i];

	for (i = 0; i < luf->r; i++) {
		luf->rows[i] = malloc(sizeof(row));
		luf->rows[i]->blocks = calloc(luf->m, sizeof(row_block));
		luf->rows[i]->m = luf->m;
	}

	for (i = 0; i < luf->n; i++) {
		SETBIT(luf->rows[i]->blocks, i, luf->c);
		if (i < a->l)
			luf->rows[i]->v = a->vars[i]->worth;
		else
			SETBIT(luf->rows[i]->blocks, a->req[i - a->l], luf->c);
	}

	a->luf = luf;
}

void compute_payment(agent *a) {

	a->payment = max(a->pf->rows, 0, a->pf->r);
}
