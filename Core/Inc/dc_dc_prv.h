#ifndef INC_DC_DC_PRV_H_
#define INC_DC_DC_PRV_H_

#include "main.h"       // Project-specific definitions (e.g., mainHRTIM_PERIOD)
#include <stdbool.h>    // Enables use of 'bool', 'true', 'false'

// Structure to store buck and boost duty cycles
typedef struct xDUTY_CYCLE {
    uint32_t ulDA;      // Buck duty cycle
    uint32_t ulDB;      // Boost duty cycle
} DutyCycle_t;

// Structure to hold the current mode and switch flag
typedef struct xDC_DC_MODE {
    uint32_t ulMode;    // 0 = BUCK, 1 = BOOST, 2 = MIXED
    bool xSwitch;       // Indicates whether a mode switch is needed
} DcDcMode_t;

// Output structure of PI controller
typedef struct xPIOUT {
    uint32_t ulD;       // PI output duty cycle
    bool xTopHit;       // Hit upper limit
    bool xBottomHit;    // Hit lower limit
} PiOut_t;

// Integral term clamp for PI controller (half the PWM period)
#define dcdcINT_TERM_LIMIT ((int32_t)(mainHRTIM_PERIOD / 2))

// Buck duty cycle range
#define dcdcMIN_D_A        ((uint32_t)1500u)
#define dcdcMAX_D_A        ((uint32_t)8000u)

// Boost duty cycle range
#define dcdcMIN_D_B        ((uint32_t)500u)
#define dcdcMAX_D_B        ((uint32_t)8000u)

// Boost limits in mixed mode
#define dcdcMIN_D_B_MIXED  ((uint32_t)500u)
#define dcdcMAX_D_B_MIXED  ((uint32_t)4500u)

// Max counter for some internal logic (e.g., mode switch threshold)
#define dcdcMAX_CNT        ((uint32_t)1000u)

// Mode identifiers
#define dcdcMODE_BUCK      ((uint32_t)0u)
#define dcdcMODE_BOOST     ((uint32_t)1u)
#define dcdcMODE_MIXED     ((uint32_t)2u)

// Internal function to set buck and boost duty cycles
extern void vSetDutyCycle(const uint32_t ulDA, const uint32_t ulDB);

// Return current duty cycle configuration
extern DutyCycle_t xGetDutyCycle(void);

// Determine if a mode switch is needed based on top/bottom counters
extern DcDcMode_t xPiEvalModeSwitch(const uint32_t ulModeOld, const uint32_t ulCntTop, const uint32_t ulCntBottom);

// Clamp the integral term of the PI controller
extern int32_t lPiLimitIntTerm(const int32_t lIntTermOld);

// Clamp the final PI output and return status of limits
extern PiOut_t ulPiLimitPiOut(const uint32_t ulPiOutDOld, const uint32_t ulMode);

#endif /* INC_DC_DC_PRV_H_ */
