#include "listsort.h"

struct list *sort_list(struct list *h, size_t *(*cmp)(struct list *, struct list *)) {
	struct list *p, *q, *e, *t;
	size_t is, ps, qs, n, i;

	if (!h) return NULL;

	is = 1;

	while (1) {
		p = h;
		h = NULL;
		t = NULL;

		n = 0; /* count number of merges we do in this pass */

		while (p) {
			n++; /* there exists a merge to be done */
			/* step `insize' places along from p */
			q = p;
			ps = 0;
			for (i = 0; i < is; i++) {
				ps++;
				q = q->n;
				if (!q) break;
			}

			/* if q hasn't fallen off end, we have two lists to merge */
			qs = is;

			/* now we have two lists; merge them */
			while (ps > 0 || (qs > 0 && q)) {

				/* decide whether next element of merge comes from p or q */
				if (ps == 0) {
					/* p is empty; e must come from q. */
					e = q;
					q = q->n;
					qs--;
				} else if (qs == 0 || !q) {
					/* q is empty; e must come from p. */
					e = p;
					p = p->n;
					ps--;
				} else if (cmp(p, q) <= 0) {
					/* First element of p is lower (or same);
					 * e must come from p. */
					e = p;
					p = p->n;
					ps--;
				} else {
					/* First element of q is lower; e must come from q. */
					e = q;
					q = q->n;
					qs--;
				}

				/* add the next element to the merged list */
				if (t) {
					t->n = e;
				} else {
					h = e;
				}
				t = e;
			}

			/* now p has stepped `insize' places along, and q has too */
			p = q;
		}
		t->n = NULL;

		/* If we have done only one merge, we're finished. */
		if (n <= 1) /* allow for nmerges==0, the empty list case */
		return h;

		/* Otherwise repeat, merging lists twice the size */
		is *= 2;
	}
}
