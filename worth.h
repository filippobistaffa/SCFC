#ifndef WORTH_H_
#define WORTH_H_

#include "scfc.h"
#include <stdio.h>

/**
 * Input Format Info
 */

#define SLOTS_PER_DAY 48
#define VALUE_LENGTH 9
#define DATE_LENGTH 10
#define USER_LENGTH 8
#define SEPARATOR 1

/**
 * Cost Parameters
 */

#define FORWARD_MARKET_COST 1
#define DAY_AHEAD_MARKET_COST 3

#define OFFSET (USER_LENGTH + SEPARATOR + DATE_LENGTH)
#define LINE_LENGTH (OFFSET + (SEPARATOR + VALUE_LENGTH) * SLOTS_PER_DAY + 1)

void free_data(value **data, size_t users);
value **read_data(char *filename, size_t users, size_t days);
value compute_ldf(variable *v, value **data, size_t users, size_t days);
value compute_worth(variable *v, value **data, size_t users, size_t days);

#endif /* WORTH_H_ */
