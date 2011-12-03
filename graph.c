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

	variable *v = malloc(sizeof(variable));
	v->n = 0;
	v->agents = malloc(MAX_AGENTS * sizeof(agent *));

	size_t i, n, a = 0;

	while ((n = fread(buf, BLOCK, BLOCK_NUMBER, f))) {
		for (i = 0; i < n; i++) {

			memcpy(id, buf + i * BLOCK, BLOCK - 1);
			v->agents[v->n++] = agents[atoi(id)];

			if (buf[(i + 1) * BLOCK - 1] == '\n' || buf[(i + 1) * BLOCK - 1] == '\r') {

				v->agents = realloc(v->agents, v->n * sizeof(agent *));

				if (agents[a]->vars)
					add(LIST(agents[a]->vars), v);
				else {
					agents[a]->vars = calloc(1, sizeof(var_list));
					agents[a]->vars->v = v;
				}

				v = malloc(sizeof(variable));
				v->n = 0;
				v->agents = malloc(MAX_AGENTS * sizeof(agent *));

				if (buf[(i + 1) * BLOCK - 1] == '\r') a++;
			}
		}

		if (n < BLOCK_NUMBER) break;
	}

	free(id);
	free(buf);
	fclose(f);
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

int compare_agents(struct list *a, struct list *b) {

	return AGENT_LIST(b)->a->d - AGENT_LIST(a)->a->d;
}

void *compute_dfs(void *d) {

	dfs_data *data = (dfs_data *) d;
	agent *a = data->a;

	if (!a->r) {

		wait_token(a, data->cond, data->mutex);
		a->p = a->sender;
		a->l = a->p->l + 1;
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
			else {
				c->pch = calloc(1, sizeof(agent_list));
				c->pch->a = a;
			}

			i->a->nv = AGENT_LIST(remove_item(LIST(i->a->nv), a));
			i = i->n;
		}
	}

	a->nv = AGENT_LIST(sort_list(LIST(a->nv), compare_agents));

	if (a->token)
		add(LIST(a->token), a);
	else {
		a->token = calloc(1, sizeof(agent_list));
		a->token->a = a;
	}

	while (a->nv) {

		child *ch = calloc(1, sizeof(child));
		ch->a = a->nv->a;

		if (a->ch)
			add(LIST(a->ch), ch);
		else {
			a->ch = calloc(1, sizeof(ch_list));
			a->ch->c = ch;
		}

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

void dfs(agent **agents, size_t n) {

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	pthread_t threads[n];
	dfs_data *data[n];
	void *status;
	size_t i;

	for (i = 0; i < n; i++) {

		data[i] = malloc(sizeof(dfs_data));
		data[i]->a = agents[i];
		data[i]->cond = &cond;
		data[i]->mutex = &mutex;
		pthread_create(&threads[i], NULL, compute_dfs, data[i]);
	}

	for (i = 0; i < n; i++)
		pthread_join(threads[i], &status);
}
