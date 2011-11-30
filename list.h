#ifndef LIST_H_
#define LIST_H_

#include "scfc.h"

struct list {

	void *item;
	list *next;
};

void free_list(list *h);

#endif /* LIST_H_ */
