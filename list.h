#ifndef LIST_H_
#define LIST_H_

#include "scfc.h"

#define TUPLE_LIST(L) ((tuple_list *)(L))
#define AGENT_LIST(L) ((agent_list *)(L))
#define VAR_LIST(L) ((var_list *)(L))
#define LIST(L) ((struct list *)(L))
#define CH_LIST(L) ((ch_list *)(L))

struct tuple_list {

	tuple *t;
	tuple_list *n;
};

struct agent_list {

	agent *a;
	agent_list *n;
};

struct var_list {

	variable *v;
	var_list *n;
};

struct ch_list {

	child *c;
	ch_list *n;
};

struct list {

	void *i;
	struct list *n;
};

void free_list(struct list *h);
size_t list_size(struct list *h);
struct list *create_list(void *i);
void add(struct list *h, void *i);
struct list *get_last(struct list *h);
struct list *copy_list(struct list *h);
struct list *get(struct list *h, size_t i);
size_t equals(struct list *h, struct list *k);
struct list *find_item(struct list *h, void *a);
void append_list(struct list *h, struct list *a);
struct list *remove_item(struct list *h, void *a);
void print_list(struct list *h, char *(*f)(void *));
struct list *remove_first(struct list *h, size_t i);
size_t contains_all(struct list *h, struct list *k);
struct list *remove_all(struct list *h, struct list *k);
struct list *retain_all(struct list *h, struct list *r);
size_t index_of(struct list *h, void *i, size_t(*eq)(void *, void *));

#endif /* LIST_H_ */
