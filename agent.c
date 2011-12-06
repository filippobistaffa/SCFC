#include "agent.h"

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

char *variable_to_string(void *x) {

	// XXX Works only for < 100 agents

	variable *v = (variable *) x;
	const char post_mask[] = { 0xE2, 0x82, 0x80 };
	char *str = calloc(list_size(LIST(v->agents)) * 7 + 3, 1);
	sprintf(str, "X");
	size_t j = 1;

	agent_list *agents = v->agents;

	while (agents) {

		str[j++] = '.';

		if (agents->a->id > 9) {
			memcpy(str + j, post_mask, 3);
			*(str + j + 2) |= agents->a->id / 10;
			j += 3;
		}

		memcpy(str + j, post_mask, 3);
		*(str + j + 2) |= agents->a->id % 10;
		j += 3;

		agents = agents->n;
	}

	return str;
}

char *agent_to_string(void *x) {

	// XXX Works only for < 100 agents

	char *str = calloc(9, 1);
	sprintf(str, "Agent %02zu", ((agent *) x)->id);
	return str;
}

void compute_luf(agent *a, value **data, size_t users, size_t days, value (*worth)(variable *, value **, size_t, size_t)) {

#if ALGORITHM_MESSAGES > 0
	printf("\033[1;37m[ A-%02zu ] Creating LUF Function\033[m\n", a->id);
#endif

	size_t i;
	function *luf = malloc(sizeof(function));

	luf->id = a->id;
	luf->r = luf->n = a->n;
	luf->m = CEIL((float) luf->n / BLOCK_BITSIZE);
	luf->rows = malloc(luf->r * sizeof(row *));
	luf->vars = VAR_LIST(copy_list(LIST(a->vars)));

	for (i = 0; i < luf->r; i++) {
		luf->rows[i] = calloc(1, sizeof(row));
		luf->rows[i]->blocks = calloc(luf->m, sizeof(row_block));
		luf->rows[i]->m = luf->m;
		luf->rows[i]->n = luf->n;
	}

	var_list *vars = luf->vars;

	for (i = 0; i < luf->n; i++) {
		SETBIT(luf->rows[i], i);
		if (i < a->l)
			luf->rows[i]->v = worth(vars->v, data, users, days);
		else
			SETBIT(luf->rows[i], a->req[i - a->l]);

		vars = vars->n;
	}

	if (a->req) free(a->req);

#if MEMORY_MESSAGES > 0
	printf("[MEMORY] A-%02zu LUF Function = %zu Bytes\n", a->id, size(luf));
#endif

	a->luf = luf;
}

void compute_payment(agent *a, pthread_cond_t *cond, pthread_mutex_t *mutex) {

	if (a->p)
		a->payment = max(a->pf->rows, 0, a->pf->r)->v;
	else {
		pthread_mutex_lock(mutex);
		a->assignment = max(a->pf->rows, 0, a->pf->r);
		pthread_cond_broadcast(cond);
		pthread_mutex_unlock(mutex);
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
