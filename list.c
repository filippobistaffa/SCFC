#include "list.h"

void free_list(struct list *h) {

	struct list *n = h->n;
	free(h);
	if (n) free_list(n);
}

struct list *copy_list(struct list *h) {

	struct list *c = malloc(sizeof(struct list));
	if (h->n)
		c->n = copy_list(h->n);
	else
		c->n = NULL;
	c->i = h->i;

	return c;
}

struct list *find_item(struct list *h, void *a) {

	if (h) {
		if (h->i == a)
			return h;
		else
			return find_item(h->n, a);
	}

	return NULL;
}

size_t contains_all(struct list *h, struct list *k) {

	if (k) {
		if (find_item(h, k->i))
			return contains_all(h, k->n);
		else
			return 0;
	}

	return 1;
}

struct list *remove_item(struct list *h, void *a) {

	if (h->i == a) {
		struct list *n = h->n;
		free(h);
		return n;
	} else if (h->n) h->n = remove_item(h->n, a);

	return h;
}

struct list *remove_all(struct list *h, struct list *k) {

	if (k) return remove_all(remove_item(h, k->i), k->n);
	return h;
}

void append(struct list *h, struct list *a) {

	if (h->n)
		append(h->n, a);
	else
		h->n = copy_list(a);
}
