#ifndef SCFC_H_
#define SCFC_H_

#include <stdlib.h>

#define THREADS_PER_CORE 4

/**
 * Type implementation
 */

typedef unsigned row_block;
typedef float value;

/**
 * Type definition
 */

typedef struct row row;
typedef struct function function;
typedef struct variable variable;
typedef struct sum_data sum_data;
typedef struct max_data max_data;
typedef struct shift_data shift_data;

typedef struct agent_list agent_list;
typedef struct children children;
typedef struct agent agent;

/**
 * Header files
 */

#include "agent.h"
#include "function.h"

#endif /* SCFC_H_ */
