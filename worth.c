#include "worth.h"

value compute_worth(variable *v, value **data, size_t users, size_t days) {

	size_t t;
	value w = 0, cur, min = INFINITY;

	for (t = 0; t < days * SLOTS_PER_DAY; t++) {

		cur = data[v->agents->a->id][t];
		agent_list *agents = v->agents->n;

		while (agents) {

			cur += data[agents->a->id][t];
			agents = agents->n;
		}

		if (cur < min) min = cur;
		w += cur * DAY_AHEAD_MARKET_COST;
	}

	w += min * (FORWARD_MARKET_COST - DAY_AHEAD_MARKET_COST) * SLOTS_PER_DAY * days;
	w += (double)(list_size(LIST(v->agents)) - 1) * FORWARD_MARKET_COST / users;
	w = -w;

#if WORTH_MESSAGES > 0
	char *str = variable_to_string(v);
	printf("\033[1;37m[INFO] W(%s) = %f (min = %f, malus = %f)\033[m\n", str, w, min, (double)(list_size(LIST(v->agents)) - 1) * FORWARD_MARKET_COST / users);
	free(str);
#endif

	v->w = w;
	return w;
}

value compute_ldf(variable *v, value **data, size_t users, size_t days) {

	value w;

	if (list_size(LIST(v->agents)) == 1)
		w = 1;
	else {
		value *num_maxes = calloc(users, sizeof(value));
		value num_sum = 0;
		value den_sum;
		value den_max = 0;
		size_t id, t;

		for (t = 0; t < days * SLOTS_PER_DAY; t++) {

			den_sum = 0;

			agent_list *agents = v->agents;

			while (agents) {

				id = agents->a->id;

				if (data[id][t] > num_maxes[id]) {
					num_sum += data[id][t] - num_maxes[id];
					num_maxes[id] = data[id][t];
				}

				den_sum += data[id][t];
				agents = agents->n;
			}

			den_max = den_sum > den_max ? den_sum : den_max;
		}

		free(num_maxes);
		w = num_sum / den_max + list_size(LIST(v->agents)) - 1;
	}

#if WORTH_MESSAGES > 0
	char *str = variable_to_string(v);
	printf("\033[1;37m[INFO] W(%s) = %f\033[m\n", str, w);
	free(str);
#endif

	v->w = w;
	return w;
}

value **read_data(char *filename, size_t users, size_t days) {

	value **data = malloc(users * sizeof(value *));

	char *buf, *v, *u;
	buf = malloc(LINE_LENGTH);
	v = calloc(1, VALUE_LENGTH);
	u = calloc(1, USER_LENGTH + 1);

	size_t i, d, t, k;
	FILE *f = fopen(filename, "rb");

	if (!f) {
		fprintf(stderr, "\033[1;31m[ERR!] %s Not Found\033[m\n", filename);
		free(data);
		data = NULL;
		goto end;
	}

	if (!fread(buf, LINE_LENGTH, 1, f)) {
		fprintf(stderr, "\033[1;31m[ERR!] %s Is Empty\033[m\n", filename);
		free(data);
		data = NULL;
		goto end;
	}

	for (i = 0; i < users; i++) {

		memcpy(u, buf, USER_LENGTH);
		data[i] = malloc(days * SLOTS_PER_DAY * sizeof(value));

		for (d = 0; d < days; d++) {

			k = OFFSET;
			for (t = 0; t < SLOTS_PER_DAY; t++) {
				memcpy(v, buf + k + 1, VALUE_LENGTH);
				data[i][t + SLOTS_PER_DAY * d] = atof(v);
				k += VALUE_LENGTH + 1;
			}

			if (d + 1 != days && (!fread(buf, LINE_LENGTH, 1, f) || strncmp(buf, u, USER_LENGTH))) {
				fprintf(stderr, "\033[1;31m[ERR!] Not Enough Days In %s\033[m\n", filename);
				free(data);
				data = NULL;
				goto end;
			}
		}

		if (i + 1 != users) {
			while (!strncmp(buf, u, USER_LENGTH))
				if (!fread(buf, LINE_LENGTH, 1, f)) {
					fprintf(stderr, "\033[1;31m[ERR!] Not Enough Users In %s\033[m\n", filename);
					free(data);
					data = NULL;
					goto end;
				}
		}
	}

	end: free(v);
	free(u);
	free(buf);
	if (f) fclose(f);
	return data;
}

void free_data(value **data, size_t users) {

	size_t i;
	for (i = 0; i < users; i++)
		free(data[i]);
	free(data);
}
