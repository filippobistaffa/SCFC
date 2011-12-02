#include "graph.h"
#include "list.h"

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

