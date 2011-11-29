#ifndef FUNCTION_H_
#define FUNCTION_H_

#include "scfc.h"

#include <math.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define ECHO 1

#define BITAT(X, I) (X >> I & 1)
#define VAR(F, I) (F->vars[I / F->c][I % F->c])
#define SETBIT(A, I, B) (A[I / B] |= 1 << (I % B))
#define CEIL(X) (X - (size_t)(X) > 0 ? (size_t)(X + 1) : (size_t)(X))
#define BIT(F, R, C) BITAT(((F->rows)[R]->blocks)[C / F->c], C % F->c)

struct variable {

	size_t n;
	agent **agents;
	value worth;
};

struct row {

	row_block *blocks;
	size_t m;
	value v;
};

struct function {

	/**
	 * s = bits per row block
	 * n = number of columns / variables
	 * m = number of row block
	 * r = number of rows
	 */

	size_t c, r, n, m, id;
	variable ***vars;
	row **rows;
};

struct sum_data {

	function *f1, *f2, *f3;
	pthread_mutex_t m;
	size_t a, *sh;
};

struct shift_data {

	row_block *block;
	size_t n, l;
};

struct max_data {

	function *f1, *f2;
	row_block *blocks;
	pthread_mutex_t m;
	size_t i, j;
};

int compatible(function *f1, size_t a, function *f2, size_t b, size_t *shared);
int compare_rows(const void *a, const void *b);
value max(function *f, size_t i, size_t j);
void subtract(function *f, value v);

void create_luf(agent *a);
void nuke(function *f);

#endif /* FUNCTION_H_ */
