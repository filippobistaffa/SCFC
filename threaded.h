#ifndef THREADED_H_
#define THREADED_H_

#include "scfc.h"
#include "list.h"
#include "compute.h"

#define LIMIT_MEMORY(M) (M < MAX_MEMORY ? M : MAX_MEMORY)
#define THREAD_NUMBER sysconf(_SC_NPROCESSORS_CONF) * THREADS_PER_CORE

function *joint_sum(function *f1, function *f2);
function *maximize(agent *a);
void get_arg_max(agent *a);

#endif /* THREADED_H_ */
