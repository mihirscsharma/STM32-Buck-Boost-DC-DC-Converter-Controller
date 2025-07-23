#include "timer.h"

// This variable counts "ticks" of a high-frequency timer.
// Typically, it is incremented in a timer interrupt (not shown here).
volatile uint32_t ulHighFrequencyTimerTicks;

/**
 * @brief Initialize/reset the run time stats timer counter.
 *        Called by FreeRTOS when setting up run time stats.
 */
void configureTimerForRunTimeStats(void)
{
    ulHighFrequencyTimerTicks = 0u; // Start at zero
}

/**
 * @brief Return the current tick count.
 *        Called by FreeRTOS to get the current time for profiling.
 * @return Current value of high-frequency tick counter
 */
uint32_t getRunTimeCounterValue(void)
{
    return ulHighFrequencyTimerTicks;
}
