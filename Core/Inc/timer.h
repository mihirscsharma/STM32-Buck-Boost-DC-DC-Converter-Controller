#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include <stdint.h>  // For fixed-width types like uint32_t

// Initialize/configure a high-frequency timer
// Useful for tracking execution time of tasks or run-time stats
extern void configureTimerForRunTimeStats(void);

// Return the current high-frequency counter value (ticks since start)
extern uint32_t getRunTimeCounterValue(void);

// Global timer tick counter incremented by ISR (e.g., in SysTick or timer interrupt)
// 'volatile' prevents compiler from caching its value
extern volatile uint32_t ulHighFrequencyTimerTicks;

#endif /* INC_TIMER_H_ */
