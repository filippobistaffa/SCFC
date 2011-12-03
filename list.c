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

struct list *get_last(struct list *h) {

	if (h->n) return get_last(h->n);
	return h;
}

void add(struct list *h, void *i) {

	if (h->n)
		add(h->n, i);
	else {
		h->n = calloc(1, sizeof(struct list));
		h->n->i = i;
	}
}

struct list *retain_all(struct list *h, struct list *r) {

	if (!h) return NULL;
	if (find_item(r, h->i)) {
		h->n = retain_all(h->n, r);
		return h;
	} else
		return retain_all(remove_item(h, h->i), r);
}

void print_list(struct list *h, char *(*f)(void *)) {

	if (h) {
		char *str = f(h->i);
		printf("%s ", str);
		free(str);
		print_list(h->n, f);
	}
	else puts("");
}

struct list *create_list(void *i) {

	struct list *h = calloc(1, sizeof(struct list));
	h->i = i;
	return h;
}

size_t list_size(struct list *h) {

	if (h) return 1 + list_size(h->n);
	return 0;
}

size_t equals(struct list *h, struct list *k) {

	if (h && k)	return (h->i == k->i) && equals(h->n, k->n);
}

size_t index_of(struct list *h, void *i, size_t(*eq)(void *, void *)) {

	if (eq(h->i, i)) return 0;
	return 1 + index_of(h->n, i, eq);

}
