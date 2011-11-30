#ifndef WORTH_H_
#define WORTH_H_

#include "scfc.h"
#include <stdio.h>

#define SLOTS_PER_DAY 48
#define LINE_LENGTH 404
#define VALUE_LENGTH 8
#define USER_LENGTH 8
#define OFFSET 19

#define FORWARD_MARKET_COST 1
#define DAY_AHEAD_MARKET_COST 3

void compute_worth(variable *v, value **data, size_t days, size_t n);
value **read_data(char *filename, size_t users, size_t days);

#endif /* WORTH_H_ */
