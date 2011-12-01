#ifndef LIST_H_
#define LIST_H_

#include "scfc.h"

#define AGENT_LIST(L) ((agent_list *)L)
#define VAR_LIST(L) ((var_list *)L)

struct agent_list {

	agent *a;
	agent_list *n;
};

struct var_list {

	variable *v;
	var_list *n;
};

struct list {

	void *i;
	struct list *n;
};

void free_list(struct list *h);
struct list *copy_list(struct list *h);
void append(struct list *h, struct list *a);
struct list *find_item(struct list *h, void *a);
struct list *remove_item(struct list *h, void *a);
size_t contains_all(struct list *h, struct list *k);
struct list *remove_all(struct list *h, struct list *k);

#endif /* LIST_H_ */
