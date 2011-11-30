#ifndef LIST_H_
#define LIST_H_

#include "scfc.h"

struct list {

	void *item;
	list *next;
};

void free_list(list *h);
list *copy_list(list *h);
void append(list *h, list *a);
list *find_item(list *h, void *a);
list *remove_all(list *h, list *k);
list *remove_item(list *h, void *a);
size_t contains_all(list *h, list *k);

#endif /* LIST_H_ */
