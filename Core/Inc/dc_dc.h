#ifndef INC_DC_DC_H_       // Prevent multiple inclusion of this header
#define INC_DC_DC_H_

#include <stdint.h>        // Use fixed-width types like uint32_t

// Set PWM duty cycle for Buck converter (ulDA = duty cycle value)
extern void vSetDutyCycleBuck(const uint32_t ulDA);

// Set PWM duty cycle for Boost converter (ulDB = duty cycle value)
extern void vSetDutyCycleBoost(const uint32_t ulDB);

// Set duty cycles for both Buck and Boost converters (for hybrid/mixed mode)
extern void vSetDutyCycleMixed(const uint32_t ulDA, const uint32_t ulDB);

// Main control function: regulates DC-DC output voltage
// ulVout = current measured voltage, ulVoutTarget = desired output voltage
extern void vDcDcControl(const uint32_t ulVout, const uint32_t ulVoutTarget);

// Set ADC trigger timing or threshold value
extern void vSetAdcTriggerPoint(const uint32_t ulVal);

#endif /* INC_DC_DC_H_ */  // End of include guard
