#ifndef INC_ADC_H_       // If not already defined, define the header guard
#define INC_ADC_H_

#include <stdint.h>      // Include standard integer types (e.g., uint32_t)

// Convert raw ADC input voltage (Vin) to physical voltage value
extern uint32_t ulVinRawToPhys(const uint32_t ulVinRaw);

// Convert raw ADC output voltage (Vout) to physical voltage value
extern uint32_t ulVoutRawToPhys(const uint32_t ulVoutRaw);

#endif /* INC_ADC_H_ */  // End of header guard
