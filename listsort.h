#ifndef LISTSORT_H_
#define LISTSORT_H_

#include "list.h"

struct list *sort_list(struct list *h, int(*cmp)(struct list *, struct list *));

#endif /* LISTSORT_H_ */
