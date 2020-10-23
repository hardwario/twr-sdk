#ifndef _BC_SLEEP_H
#define _BC_SLEEP_H

#include <bc_system.h>

typedef struct bc_sleep_manager {
    int disable_sleep;
} bc_sleep_manager_t;

extern bc_sleep_manager_t sleep_manager;

/**
 * Prevent the processor from sleeping
 *
 * The function sets an internal flag that will prevent the process core from
 * entering low power (sleep) modes. If called repeatedly, bc_sleep_enable must
 * be called repeatedly also to re-enable sleep modes.
 */
void bc_sleep_disable(void);

/**
 * Allow the processor to enter low power modes when idle
 *
 * Call this function after calling bc_sleep_disable to re-enable low power
 * (sleep) process modes. Must be called repeatedly if bc_sleep_disable was
 * called repeatedly to enable sleeping.
 */
void bc_sleep_enable(void);

/**
 * Put the processor to low power mode (unless disabled)
 *
 * Transition the processor core to a low power mode in the absence of work to
 * do, unless sleeping has been disabled with bc_sleep_disable. The function
 * returns immediately if sleeping has been disabled by the application.
 *
 * Sleeping is enabled by default (upon application startup).
 */
static inline void bc_sleep(void)
{
    if (sleep_manager.disable_sleep == 0) {
        bc_system_sleep();
    }
}

#endif /* _BC_SLEEP_H */