#include "worth.h"

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
