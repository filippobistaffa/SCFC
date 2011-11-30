#ifndef WORTH_H_
#define WORTH_H_

#include "scfc.h"
#include <stdio.h>

#define SLOTS_PER_DAY 48
#define LINE_LENGTH 404
#define VALUE_LENGTH 8
#define USER_LENGTH 8
#define OFFSET 19

value **read_data(char *filename, size_t users, size_t days);

#endif /* WORTH_H_ */
