#include "scfc.h"

int test(int argc, char *argv[]) {

	setlocale(LC_ALL, "");

	agent *a0 = malloc(sizeof(agent));
	agent *a1 = malloc(sizeof(agent));
	agent *a2 = malloc(sizeof(agent));

	a0->p = NULL;
	a1->p = a0;
	a2->p = a1;

	a0->id = 0;
	a1->id = 1;
	a2->id = 2;

	size_t *req1 = calloc(2, sizeof(size_t));
	size_t *req2 = calloc(2, sizeof(size_t));

	req1[1] = 1;

	a0->n = 4;
	a1->n = 4;
	a2->n = 3;

	a0->l = 4;
	a1->l = 2;
	a2->l = 1;

	a1->req = req1;
	a2->req = req2;

	variable *x0 = malloc(sizeof(variable));
	variable *x1 = malloc(sizeof(variable));
	variable *x2 = malloc(sizeof(variable));
	variable *x01 = malloc(sizeof(variable));
	variable *x02 = malloc(sizeof(variable));
	variable *x12 = malloc(sizeof(variable));
	variable *x012 = malloc(sizeof(variable));

	x0->n = 1;
	x1->n = 1;
	x2->n = 1;
	x01->n = 2;
	x02->n = 2;
	x12->n = 2;
	x012->n = 3;

	x0->agents = malloc(x0->n * sizeof(agent *));
	x1->agents = malloc(x1->n * sizeof(agent *));
	x2->agents = malloc(x2->n * sizeof(agent *));
	x01->agents = malloc(x2->n * sizeof(agent *));
	x02->agents = malloc(x02->n * sizeof(agent *));
	x12->agents = malloc(x12->n * sizeof(agent *));
	x012->agents = malloc(x012->n * sizeof(agent *));

	x0->agents[0] = a0;
	x1->agents[0] = a1;
	x2->agents[0] = a2;
	x01->agents[0] = a0;
	x01->agents[1] = a1;
	x02->agents[0] = a0;
	x02->agents[1] = a2;
	x12->agents[0] = a1;
	x12->agents[1] = a2;
	x012->agents[0] = a0;
	x012->agents[1] = a1;
	x012->agents[2] = a2;

	var_list *a0vars = calloc(1, sizeof(var_list));
	var_list *a1vars = calloc(1, sizeof(var_list));
	var_list *a2vars = calloc(1, sizeof(var_list));

	a0vars->v = x0;
	add(LIST(a0vars), x01);
	add(LIST(a0vars), x012);
	add(LIST(a0vars), x02);

	a1vars->v = x1;
	add(LIST(a1vars), x12);
	add(LIST(a1vars), x01);
	add(LIST(a1vars), x012);

	a2vars->v = x2;
	add(LIST(a2vars), x12);
	add(LIST(a2vars), x02);

	a0->vars = a0vars;
	a1->vars = a1vars;
	a2->vars = a2vars;

	value **data = read_data("/home/liquidator/20090112.csv", 3, 1);

	compute_ldf(x0, data, 3, 1);
	compute_ldf(x1, data, 3, 1);
	compute_ldf(x2, data, 3, 1);
	compute_ldf(x01, data, 3, 1);
	compute_ldf(x02, data, 3, 1);
	compute_ldf(x12, data, 3, 1);
	compute_ldf(x012, data, 3, 1);

	x0->worth = 0.1;
	x1->worth = 0.6;
	x2->worth = 0.1;
	x01->worth = 0.1;
	x02->worth = 2;
	x12->worth = 1.5;
	x012->worth = 2;

	create_luf(a0);
	create_luf(a1);
	create_luf(a2);

	function *a1msg, *a2msg;

	a2->pf = a2->luf;
	compute_payment(a2);
	a2msg = maximize(a2);
	subtract(a2msg, a2->payment);

	a1->pf = joint_sum(a1->luf, a2msg);
	compute_payment(a1);
	a1msg = maximize(a1);
	subtract(a1msg, a1->payment);

	a0->pf = joint_sum(a0->luf, a1msg);
	compute_payment(a0);

	get_arg_max(a1);
	get_arg_max(a2);

	nuke(a0->pf);
	nuke(a1->pf);
	nuke(a1msg);
	nuke(a2msg);
	nuke(a0->luf);
	nuke(a1->luf);
	nuke(a2->luf);

	printf("\033[1;32mNiente segmentation fault, l'avaressito mai dito?\033[m\n");

	return 0;
}

int main(int argc, char *argv[]) {

	size_t i, n, r = 0;
	agent **agents = read_dot("/home/liquidator/data/scalefreenetwork.dot", &n);
	read_vars("/home/liquidator/data/coalitions.txt", agents);

	for (i = 1; i < n; i++)
		if (agents[i]->d > agents[r]->d) r = i;

	agents[r]->r = 1;
	dfs(agents, n);

	for (i = 0; i < n; i++) {

		if (agents[i]->p)
			printf("Agent %zu parent = Agent %zu\n", agents[i]->id, agents[i]->p->id);
		else
			printf("Agent %zu is root\n", agents[i]->id);

		printf("Agent %zu pseudo-parents = ", agents[i]->id);
		print_list(LIST(agents[i]->pp), agent_to_string);
		puts("");

		ch_list *j = agents[i]->ch;

		while (j) {

			printf("Pseudo-children down Agent %zu = ", j->c->a->id);
			print_list(LIST(j->c->pch), agent_to_string);
			puts("");
			j = j->n;
		}
	}

	printf("\033[1;32mNiente segmentation fault, l'avaressito mai dito?\033[m\n");
	return 0;

}
