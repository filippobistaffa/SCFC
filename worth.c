#include "worth.h"

void compute_worth(variable *v, value **data, size_t users, size_t days) {

	size_t i, t;
	value w = 0, cur, min = INFINITY;

	for (t = 0; t < days * SLOTS_PER_DAY; t++) {

		cur = data[v->agents[0]->id][t];
		for (i = 1; i < v->n; i++)
			cur += data[v->agents[i]->id][t];
		if (cur < min) min = cur;
		w += cur * DAY_AHEAD_MARKET_COST;
	}

	w += min * (FORWARD_MARKET_COST - DAY_AHEAD_MARKET_COST) * SLOTS_PER_DAY * days + (v->n - 1) * FORWARD_MARKET_COST / users;
	v->worth = 1 / w;

#if ALGORITHM_MESSAGES > 0
	char *str = variable_to_string(v);
	printf("\033[1;37m[ INFO ] W(%s) = %f\033[m\n", str, v->worth);
	free(str);
#endif
}

void compute_ldf(variable *v, value **data, size_t users, size_t days) {

	if (v->n == 1) v->worth = 1;
	else {
		value *num_maxes = calloc(users, sizeof(value));
		value num_sum = 0;
		value den_sum;
		value den_max = 0;
		size_t i, t;

		for (t = 0; t < days * SLOTS_PER_DAY; t++) {

			den_sum = 0;

			for (i = 0; i < v->n; i++) {

				if (data[v->agents[i]->id][t] > num_maxes[v->agents[i]->id]) {
					num_sum += data[v->agents[i]->id][t] - num_maxes[v->agents[i]->id];
					num_maxes[v->agents[i]->id] = data[v->agents[i]->id][t];
				}

				den_sum += data[v->agents[i]->id][t];
			}

			den_max = den_sum > den_max ? den_sum : den_max;
		}

		free(num_maxes);
		v->worth = num_sum / den_max + v->n - 1;
	}

#if ALGORITHM_MESSAGES > 0
	char *str = variable_to_string(v);
	printf("\033[1;37m[ INFO ] W(%s) = %f\033[m\n", str, v->worth);
	free(str);
#endif
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
		fprintf(stderr, "\033[1;31m[ERR0R!] %s Not Found\033[m\n", filename);
		free(data);
		data = NULL;
		goto end;
	}

	if (!fread(buf, 1, LINE_LENGTH, f)) {
		fprintf(stderr, "\033[1;31m[ERR0R!] %s Is Empty\033[m\n", filename);
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

			if (d + 1 != days && (!fread(buf, 1, LINE_LENGTH, f) || strncmp(buf, u, USER_LENGTH))) {
				fprintf(stderr, "\033[1;31m[ERR0R!] Not Enough Days In %s\033[m\n", filename);
				free(data);
				data = NULL;
				goto end;
			}
		}

		if (i + 1 != users) {
			while (!strncmp(buf, u, USER_LENGTH))
				if (!fread(buf, 1, LINE_LENGTH, f)) {
					fprintf(stderr, "\033[1;31m[ERR0R!] Not Enough Users In %s\033[m\n", filename);
					free(data);
					data = NULL;
					goto end;
				}
		}
	}

	end: free(v);
	free(u);
	free(buf);
	fclose(f);
	return data;
}

void free_data(value **data, size_t users) {

	size_t i;
	for (i = 0; i < users; i++)
		free(data[i]);
	free(data);
}
