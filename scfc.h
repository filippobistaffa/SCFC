#ifndef SCFC_H_
#define SCFC_H_

#include <stdlib.h>

/**
 * Logging Flags
 */

#define ALGORITHM_MESSAGES 1
#define THREAD_MESSAGES 1
#define MEMORY_MESSAGES 0
#define WORTH_MESSAGES 0

/**
 * Constants
 */

#define THREADS_PER_CORE 1.5
#define MAX_STRING_SIZE 1048576

/**
 * Type Definition
 */

typedef struct function function;
typedef struct variable variable;
typedef struct row row;

typedef struct shift_data shift_data;
typedef struct sum_data sum_data;
typedef struct max_data max_data;
typedef struct arg_data arg_data;
typedef struct msg_data msg_data;

typedef struct tuple_list tuple_list;
typedef struct agent_list agent_list;
typedef struct var_list var_list;
typedef struct ch_list ch_list;
typedef struct agent agent;
typedef struct child child;
typedef struct tuple tuple;

/**
 * Type Implementation
 */

typedef unsigned row_block;
typedef float value;
typedef agent item;

/**
 * Header Files
 */

#include "function.h"
#include "agent.h"
#include "graph.h"
#include "list.h"

#endif /* SCFC_H_ */
