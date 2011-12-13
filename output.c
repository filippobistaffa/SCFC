#include "output.h"

void recursive_graph(agent *a, FILE *f) {

	size_t i;

	INDENT(0);
	fprintf(f, "<agent>\n");

	INDENT(1);
	fprintf(f, "<id>%zu</id>\n", a->id);

	if (a->pp) {

		INDENT(1);
		fprintf(f, "<pseudoparents>\n");
		agent_list *pp = a->pp;

		while (pp) {
			INDENT(2);
			fprintf(f, "<agent>\n");
			INDENT(3);
			fprintf(f, "<id>%zu</id>\n", pp->a->id);
			INDENT(2);
			fprintf(f, "</agent>\n");
			pp = pp->n;
		}

		INDENT(1);
		fprintf(f, "</pseudoparents>\n");
	}

	if (a->ch) {

		INDENT(1);
		fprintf(f, "<children>\n");

		ch_list *children = a->ch;

		while (children) {
			recursive_graph(children->c->a, f);
			children = children->n;
		}

		INDENT(1);
		fprintf(f, "</children>\n");
	}

	INDENT(0);
	fprintf(f, "</agent>\n");
}

void recursive_coalition(agent *a, size_t *p, FILE *f) {

	if (!p[a->id]) {

		fprintf(f, "\t\t<coalition worth=\"%f\">\n", a->v->w);
		agent_list *agents = a->v->agents;

		while (agents) {
			fprintf(f, "\t\t\t<agent payment=\"%f\" gain=\"%f\">%zu</agent>\n", agents->a->payment, agents->a->payment - agents->a->single, agents->a->id);
			p[agents->a->id] = 1;
			agents = agents->n;
		}

		fprintf(f, "\t\t</coalition>\n");
	}

	ch_list *children = a->ch;

	while (children) {
		recursive_coalition(children->c->a, p, f);
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

	fprintf(f, "<scf>\n\t<graph>\n");
	recursive_graph(root, f);
	fprintf(f, "\t</graph>\n\t<solution>\n");
	recursive_coalition(root, p, f);
	fprintf(f, "\t</solution>\n</scf>\n");
	fclose(f);
	free(p);
	return 0;
}
