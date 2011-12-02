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

struct list *remove_first(struct list *h, size_t i) {

	if (!i) return h;
	struct list *ret, *temp;
	temp = get(h, i - 1);
	ret = temp->n;
	temp->n = NULL;
	free_list(h);
	return ret;
}

void append_list(struct list *h, struct list *a) {

	if (h->n)
		append_list(h->n, a);
	else
		h->n = copy_list(a);
}

struct list *get(struct list *h, size_t i) {

	if (!h) return NULL;
	if (!i) return h;
	return get(h->n, i - 1);
}

void add(struct list *h, void *i) {

	if (h->n)
		add(h->n, i);
	else {
		h->n = malloc(sizeof(struct list));
		h->n->n = NULL;
		h->n->i = i;
	}
}

void print_list(struct list *h, char *(*f)(void *)) {

	if (h) {
		char *str = f(h->i);
		printf("%s\n", str);
		free(str);
		print_list(h->n, f);
	}
}
