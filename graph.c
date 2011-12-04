#include "graph.h"

void add_neighbor(agent **agents, size_t a, size_t b) {

	if (agents[a]->ngh) {

		add(LIST(agents[a]->ngh), agents[b]);
		add(LIST(agents[a]->nv), agents[b]);

	} else {

		agents[a]->ngh = calloc(1, sizeof(agent_list));
		agents[a]->ngh->a = agents[b];
		agents[a]->nv = calloc(1, sizeof(agent_list));
		agents[a]->nv->a = agents[b];
	}

	agents[a]->d++;
}

agent **read_dot(char *filename, size_t *n) {

	FILE *f = fopen(filename, "r");

	if (!f) {
		fprintf(stderr, "\033[1;31m[ERR0R!] %s Not Found\033[m\n", filename);
		fclose(f);
		return NULL;
	}

	agent **agents = malloc(MAX_AGENTS * sizeof(agent *));
	char *i, *j, *h, *k, line[LINE];
	k = malloc(3);
	h = malloc(3);
	size_t a, b, l;
	*n = 0;

	while (fgets(line, LINE, f)) {
		if (strstr(line, "{") || strstr(line, "}") || strstr(line, "label")) continue;
		if ((i = strstr(line, "\" -- \""))) {

			l = (size_t) (i - (j = strchr(line, '"')) - 1);
			memcpy(h, j + 1, l);
			h[l] = '\0';

			l = (size_t) (strchr(i + 6, '"') - (i + 6));
			memcpy(k, i + 6, l);
			k[l] = '\0';

			a = atoi(h);
			b = atoi(k);

			add_neighbor(agents, a, b);
			add_neighbor(agents, b, a);

		} else {
			agents[*n] = calloc(1, sizeof(agent));
			agents[*n]->id = *n;
			(*n)++;
		}
	}

	free(h);
	free(k);
	fclose(f);
	return realloc(agents, *n * sizeof(agent));
}

void read_vars(char *filename, agent **agents) {

	FILE *f = fopen(filename, "r");

	if (!f) {
		fprintf(stderr, "\033[1;31m[ERR0R!] %s Not Found\033[m\n", filename);
		fclose(f);
		return;
	}

	char *id = calloc(1, BLOCK);
	char *buf = malloc(BLOCK * BLOCK_NUMBER);

	variable *v = calloc(1, sizeof(variable));
	size_t i, n, a = 0;

	while ((n = fread(buf, BLOCK, BLOCK_NUMBER, f))) {
		for (i = 0; i < n; i++) {

			memcpy(id, buf + i * BLOCK, BLOCK - 1);

			if (v->agents)
				add(LIST(v->agents), agents[atoi(id)]);
			else
				v->agents = AGENT_LIST(create_list(agents[atoi(id)]));

			if (buf[(i + 1) * BLOCK - 1] == '\n' || buf[(i + 1) * BLOCK - 1] == '\r') {

				if (agents[a]->vars)
					add(LIST(agents[a]->vars), v);
				else
					agents[a]->vars = VAR_LIST(create_list(v));

				v = calloc(1, sizeof(variable));

				if (buf[(i + 1) * BLOCK - 1] == '\r') a++;
			}
		}

		if (n < BLOCK_NUMBER) break;
	}

	free(id);
	free(buf);
	fclose(f);
}

agent_list *compute_pt(agent *a) {

	agent_list *pt = AGENT_LIST(create_list(a));
	ch_list *children = a->ch;

	while (children) {

		children->c->a->pt = compute_pt(children->c->a);
		append_list(LIST(pt), LIST(children->c->a->pt));
		children = children->n;
	}

	return pt;
}

void wait_token(agent *a, pthread_cond_t *cond, pthread_mutex_t *mutex) {

	pthread_mutex_lock(mutex);
	while (!a->token)
		pthread_cond_wait(cond, mutex);
	pthread_mutex_unlock(mutex);
}

void send_token(agent *a, agent *b, pthread_cond_t *cond, pthread_mutex_t *mutex) {

	pthread_mutex_lock(mutex);
	b->sender = a;
	b->token = a->token;
	a->token = NULL;
	pthread_cond_broadcast(cond);
	pthread_mutex_unlock(mutex);
}

int compare_degree(struct list *a, struct list *b) {

	return AGENT_LIST(b)->a->d - AGENT_LIST(a)->a->d;
}

void *compute_dfs(void *d) {

	msg_data *data = (msg_data *) d;
	agent *a = data->a;

	if (!a->r) {

		wait_token(a, data->cond, data->mutex);
		a->p = a->sender;
		a->l = a->p->l + 1;
		a->ch_id = a->p->ch_n - 1;
		a->pp = AGENT_LIST(retain_all(copy_list(LIST(a->token)), LIST(a->ngh)));
		free_list(LIST(a->ngh));
		a->nv = AGENT_LIST(remove_all(LIST(a->nv), LIST(a->pp)));
		a->pp = AGENT_LIST(remove_item(LIST(a->pp), a->p));

		agent_list *i = a->pp;
		child *c;

		while (i) {

			c = CH_LIST(get_last(LIST(i->a->ch)))->c;

			if (c->pch)
				add(LIST(c->pch), a);
			else
				c->pch = AGENT_LIST(create_list(a));

			i->a->nv = AGENT_LIST(remove_item(LIST(i->a->nv), a));
			i = i->n;
		}
	}

	a->nv = AGENT_LIST(sort_list(LIST(a->nv), compare_degree));

	if (a->token)
		add(LIST(a->token), a);
	else
		a->token = AGENT_LIST(create_list(a));

	while (a->nv) {

		child *ch = calloc(1, sizeof(child));
		ch->a = a->nv->a;
		a->ch_n++;

		if (a->ch)
			add(LIST(a->ch), ch);
		else
			a->ch = CH_LIST(create_list(ch));

		send_token(a, a->nv->a, data->cond, data->mutex);
		a->nv = AGENT_LIST(remove_item(LIST(a->nv), a->nv->a));
		wait_token(a, data->cond, data->mutex);
	}

	a->token = AGENT_LIST(remove_item(LIST(a->token), a));
	if (!a->r) send_token(a, a->p, data->cond, data->mutex);

#if ALGORITHM_MESSAGES > 0
	printf("\033[1;35m[ A-%02zu ] DFS Successfully Completed\033[m\n", a->id);
#endif

	free(data);
	pthread_exit(NULL);
}

void wait_req_msg(agent *a, pthread_cond_t *cond, pthread_mutex_t *mutex) {

	pthread_mutex_lock(mutex);
	while (!a->p->req_msgs || !a->p->req_msgs[a->ch_id])
		pthread_cond_wait(cond, mutex);
	pthread_mutex_unlock(mutex);
}

void send_req_msg(agent *a, size_t id, tuple_list *msg, pthread_cond_t *cond, pthread_mutex_t *mutex) {

	pthread_mutex_lock(mutex);
	a->req_msgs[id] = msg;
	pthread_cond_broadcast(cond);
	pthread_mutex_unlock(mutex);
}

void *compute_vars(void *d) {

	msg_data *data = (msg_data *) d;
	agent *a = data->a;
	size_t i = 0;

	var_list *non_local = NULL;

	agent_list *inter;
	tuple_list *tuples;
	a->req_msgs = calloc(a->ch_n, sizeof(tuple_list *));

	if (a->p) {

#if ALGORITHM_MESSAGES > 0
		printf("\033[1;31m[ A-%02zu ] Waiting Require Message\033[m\n", a->id);
#endif
		wait_req_msg(a, data->cond, data->mutex);

#if ALGORITHM_MESSAGES > 0
		printf("\033[1;35m[ A-%02zu ] Require Message Received\033[m\n", a->id);
#endif
		a->req = malloc(1000 * MAX_AGENTS * sizeof(size_t));
		tuples = a->p->req_msgs[a->ch_id];

		while (tuples) {

			if (find_item(LIST(tuples->t->agents), a)) {

				size_t m, j = 0, size, max_size = 0;
				agent_list *max_inter = NULL;
				var_list *vars = a->vars;

				while (vars) {

					if (contains_all(LIST(tuples->t->agents), LIST(vars->v->agents))) {
						inter = AGENT_LIST(retain_all(copy_list(LIST(vars->v->agents)), LIST(tuples->t->agents)));
						if ((size = list_size(LIST(inter))) > max_size) {
							if (max_size) free_list(LIST(max_inter));
							max_inter = inter;
							m = j;
						}
					}

					vars = vars->n;
					j++;
				}

				a->req[i++] = m;

				if (non_local)
					add(LIST(non_local), tuples->t->var);
				else
					non_local = VAR_LIST(create_list(tuples->t->var));

				tuples->t->agents = AGENT_LIST(remove_all(LIST(tuples->t->agents), LIST(max_inter)));
			}

			free(tuples->t);
			tuples = tuples->n;
		}

		free_list(LIST(a->p->req_msgs[a->ch_id]));
		a->req = realloc(a->req, i * sizeof(size_t));

#if MEMORY_MESSAGES > 0
		printf("[MEMORY] A-%02zu Require Function = %zu Bytes\n", a->id, i * sizeof(size_t));
#endif
	}

	tuple *t;
	tuple_list *msg, *req_msg = NULL;
	var_list *vars = a->vars;

	while (vars) {

		t = malloc(sizeof(tuple));
		t->var = vars->v;
		t->agents = vars->v->agents;

		if (req_msg)
			add(LIST(req_msg), t);
		else
			req_msg = TUPLE_LIST(create_list(t));

		vars = vars->n;
	}

	get_last(LIST(a->vars))->n = LIST(non_local);
	ch_list *children = a->ch;
	i = 0;

	while (children) {

		tuples = req_msg;
		msg = NULL;

		while (tuples) {

			inter = AGENT_LIST(retain_all(copy_list(LIST(tuples->t->agents)), LIST(children->c->a->pt)));

			if (inter) {

				t = malloc(sizeof(tuple));
				t->var = tuples->t->var;
				t->agents = inter;

				if (msg)
					add(LIST(msg), t);
				else
					msg = TUPLE_LIST(create_list(t));
			}

			tuples = tuples->n;
		}

#if ALGORITHM_MESSAGES > 0
		printf("\033[1;33m[ A-%02zu ] Sending Require Message To A-%02zu\033[m\n", a->id, children->c->a->id);
#endif
		send_req_msg(a, i++, msg, data->cond, data->mutex);
		children = children->n;
	}

	tuples = req_msg;
	while (tuples) {
		free(tuples->t);
		tuples = tuples->n;
	}
	free_list(LIST(req_msg));

	a->dem_msgs = calloc(a->ch_n, sizeof(function *));
	create_luf(a);

	free(data);
	pthread_exit(NULL);
}

