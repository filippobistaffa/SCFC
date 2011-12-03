#include "listsort.h"

struct list *sort_list(struct list *h, int(*cmp)(struct list *, struct list *)) {

	struct list *p, *q, *e, *t;
	size_t is, ps, qs, n, i;

	if (!h) return NULL;
	is = 1;

	while (1) {

		n = 0;
		p = h;
		h = NULL;
		t = NULL;

		while (p) {

			n++;
			q = p;
			ps = 0;

			for (i = 0; i < is; i++) {
				ps++;
				q = q->n;
				if (!q) break;
			}

			qs = is;

			while (ps > 0 || (qs > 0 && q)) {

				if (ps == 0) {
					e = q;
					q = q->n;
					qs--;
				} else if (qs == 0 || !q) {
					e = p;
					p = p->n;
					ps--;
				} else if (cmp(p, q) <= 0) {
					e = p;
					p = p->n;
					ps--;
				} else {
					e = q;
					q = q->n;
					qs--;
				}

				if (t)
					t->n = e;
				else
					h = e;

				t = e;
			}

			p = q;
		}

		t->n = NULL;
		if (n <= 1) return h;
		is *= 2;
	}
}
