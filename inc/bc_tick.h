#ifndef _BC_TICK_H
#define _BC_TICK_H

#include <bc_common.h>

#define BC_TICK_INFINITY 0xffffffff

typedef uint64_t bc_tick_t;

bc_tick_t bc_tick_get(void);

#endif /* _BC_TICK_H */
