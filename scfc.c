#include "scfc.h"

void run_agents(agent **agents, size_t n, void *(*routine)(void *)) {

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	pthread_t threads[n];
	msg_data *data[n];
	void *status;
	size_t i;

	for (i = 0; i < n; i++) {

		data[i] = malloc(sizeof(msg_data));
		data[i]->a = agents[i];
		data[i]->cond = &cond;
		data[i]->mutex = &mutex;
		pthread_create(&threads[i], NULL, routine, data[i]);
	}

	for (i = 0; i < n; i++)
		pthread_join(threads[i], &status);
}

int main(int argc, char *argv[]) {

	size_t i, n, r = 0;
	agent **agents;
	value **data;

	if (!(agents = read_dot("/home/liquidator/scalefreenetwork.dot", &n))) return 1;
	if (read_vars("/home/liquidator/coalitions.txt", agents)) return 1;

	for (i = 1; i < n; i++)
		if (agents[i]->d > agents[r]->d) r = i;

	agents[r]->r = 1;
	run_agents(agents, n, compute_dfs);

	agents[r]->pt = compute_pt(agents[r]);
	run_agents(agents, n, compute_vars);

	if (!(data = read_data("/home/liquidator/20090112.csv", n, 1))) return 1;

	for (i = 0; i < n; i++)
		compute_luf(agents[i], data, n, 1, compute_ldf);

	free_data(data, n);
	run_agents(agents, n, compute_scf);

	for (i = 0; i < n; i++)
		nuke(agents[i]->pf);

	write_xml_output(agents[r], n, "/home/liquidator/output.xml");

	printf("\033[1;32mNiente segmentation fault, l'avaressito mai dito?\033[m\n");
	return 0;
}
