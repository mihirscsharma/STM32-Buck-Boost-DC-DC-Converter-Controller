#ifndef INC_HW_H_
#define INC_HW_H_

#include <stdint.h>    // For fixed-width integer types like uint32_t

// Measured 3.3V rail voltage in millivolts (used for ADC calculations)
#define hwREAL_3V3          ((uint32_t)3305u)  // 3.305V

// Real voltage divider ratio for VIN:
// Theoretical: (6.8k / (6.8k + 27k)) * 10000 = 2012
// Measured/calibrated: 1970 (i.e., 0.1970)
// Refer to ST Application Note AN4449 for calibration method
#define hwREAL_VIN_R_RATIO  ((uint32_t)1970u)

// Real voltage divider ratio for VOUT:
// Theoretical: (3.3k / (3.3k + 13.3k)) * 10000 = 1988
// Measured/calibrated: 1978 (i.e., 0.1978)
#define hwREAL_VOUT_R_RATIO ((uint32_t)1978u)

// Memory section attribute: place variable in Core-Coupled RAM (CCMRAM)
// Useful for storing fast-access variables (like control loop buffers)
#define hwCCMRAM            __attribute__((section(".ccmram")))

#endif /* INC_HW_H_ */
