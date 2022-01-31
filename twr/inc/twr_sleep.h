#ifndef _TWR_SLEEP_H
#define _TWR_SLEEP_H

#include <twr_system.h>

typedef struct twr_sleep_manager {
    int disable_sleep_semaphore;
} twr_sleep_manager_t;

extern twr_sleep_manager_t sleep_manager;

/**
 * Prevent the processor from sleeping
 *
 * The function sets an internal flag that will prevent the process core from
 * entering low power (sleep) modes. If called repeatedly, twr_sleep_enable must
 * be called repeatedly also to re-enable sleep modes.
 */
void twr_sleep_disable(void);

/**
 * Allow the processor to enter low power modes when idle
 *
 * Call this function after calling twr_sleep_disable to re-enable low power
 * (sleep) process modes. Must be called repeatedly if twr_sleep_disable was
 * called repeatedly to enable sleeping.
 */
void twr_sleep_enable(void);

/**
 * Put the processor to low power mode (unless disabled)
 *
 * Transition the processor core to a low power mode in the absence of work to
 * do, unless sleeping has been disabled with twr_sleep_disable. The function
 * returns immediately if sleeping has been disabled by the application.
 *
 * Sleeping is enabled by default (upon application startup).
 */
static inline void twr_sleep(void)
{
    if (sleep_manager.disable_sleep_semaphore == 0) {
        twr_system_sleep();
    }
}

#endif /* _TWR_SLEEP_H */
