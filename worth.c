#include "worth.h"

void compute_worth(variable *v, value **data, size_t days, size_t n) {

	size_t i, j;
	value w = 0, cur, min = INFINITY;

	for (j = 0; j < days * SLOTS_PER_DAY; j++) {

		cur = data[v->agents[0]->id][j];
		for (i = 1; i < v->n; i++)
			cur += data[v->agents[i]->id][j];
		if (cur < min) min = cur;
		w += cur * DAY_AHEAD_MARKET_COST;
	}

	w += min * (FORWARD_MARKET_COST - DAY_AHEAD_MARKET_COST) * SLOTS_PER_DAY * days + (v->n - 1) * FORWARD_MARKET_COST / n;
	v->worth = 1 / w;

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

	if (!fread(buf, 1, LINE_LENGTH, f)) {
		fprintf(stderr, "\033[1;31m[ERR0R!] %s Not Found\033[m\n", filename);
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
				memcpy(v, buf + k + 1, VALUE_LENGTH - 1);
				data[i][t + SLOTS_PER_DAY * d] = atof(v);
				k += VALUE_LENGTH;
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
