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
	agent **agents = read_dot("/home/liquidator/data/scalefreenetwork.dot", &n);
	read_vars("/home/liquidator/data/coalitions.txt", agents);

	for (i = 1; i < n; i++)
		if (agents[i]->d > agents[r]->d) r = i;

	agents[r]->r = 1;
	run_agents(agents, n, compute_dfs);

	agents[r]->pt = compute_pt(agents[r]);
	run_agents(agents, n, compute_vars);

	printf("\033[1;32mNiente segmentation fault, l'avaressito mai dito?\033[m\n");
	return 0;
}
