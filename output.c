#include "output.h"

void recursive_print(agent *a, size_t *p, FILE *f) {

	if (!p[a->id]) {

		fprintf(f, "\t<coalition worth=\"%f\">\n", a->v->w);
		agent_list *agents = a->v->agents;

		while (agents) {
			fprintf(f, "\t\t<agent payment=\"%f\" gain=\"%f\">%zu</agent>\n", agents->a->payment, agents->a->payment - agents->a->single, agents->a->id);
			p[agents->a->id] = 1;
			agents = agents->n;
		}

		fprintf(f, "\t</coalition>\n");
	}

	ch_list *children = a->ch;

	while (children) {
		recursive_print(children->c->a, p, f);
		children = children->n;
	}
}

size_t write_xml_output(agent *root, size_t users, char *filename) {

	FILE *f = fopen(filename, "w");

	if (!f) {
		fprintf(stderr, "\033[1;31m[ERR!] Cannot Open %s\033[m\n", filename);
		return 1;
	}

	size_t *p = calloc(users, sizeof(size_t));
	fprintf(f, XML_HEADER);
	fprintf(f, "<solution>\n");
	recursive_print(root, p, f);
	fprintf(f, "</solution>\n");
	fclose(f);
	free(p);
	return 0;
}
