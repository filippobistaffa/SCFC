#include "list.h"

void free_list(list *h) {

	list *n = h->next;
	free(h);
	if (n) free_list(n);
}

list *copy_list(list *h) {

	list *c = malloc(sizeof(list));
	if (h->next)
		c->next = copy_list(h->next);
	else
		c->next = NULL;
	c->item = h->item;

	return c;
}

list *find_item(list *h, void *a) {

	if (h) {
		if (h->item == a)
			return h;
		else
			return find_item(h->next, a);
	}

	return NULL;
}

list *remove_item(list *h, void *a) {

	if (h->item == a) {
		list *n = h->next;
		free(h);
		return n;
	} else if (h->next) h->next = remove_item(h->next, a);

	return h;
}

void append(list *h, list *a) {

	if (h->next)
		append(h->next, a);
	else
		h->next = copy_list(a);
}
