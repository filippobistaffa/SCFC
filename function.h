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

#define BITAT(X, I) ((X) >> (I) & 1)
#define BLOCK_BITSIZE (sizeof(row_block) << 3)
#define VAR(F, I) ((F)->vars[(I) / BLOCK_BITSIZE][(I) % BLOCK_BITSIZE])
#define CEIL(X) (X - (size_t)(X) > 0 ? (size_t)(X + 1) : (size_t)(X))
#define SETBIT(R, I) (((R)->blocks)[(I) / BLOCK_BITSIZE] |= 1 << ((I) % BLOCK_BITSIZE))
#define GETBIT(R, I) BITAT(((R)->blocks)[(I) / BLOCK_BITSIZE], (I) % BLOCK_BITSIZE)

struct variable {

	size_t n;
	agent **agents;
	value worth;
};

struct row {

	row_block *blocks;
	size_t n, m;
	value v;
};

struct function {

	/**
	 * n = number of columns / variables
	 * m = number of row block
	 * r = number of rows
	 */

	size_t r, n, m, id;
	variable ***vars;
	row **rows;
};

int compare_rows(const void *a, const void *b);
int compatible(row *r1, row *r2, size_t *sh);
row *max(row **rows, size_t i, size_t j);
void subtract(function *f, value v);
void nuke(function *f);
size_t size(function *f);

#endif /* FUNCTION_H_ */
