/**
 * @file adc.c
 * @brief Converts raw ADC readings into physical voltages (mV)
 */

#include <stdint.h>
#include "hw.h"     // Contains real voltage divider ratios and Vref
#include "pd.h"     // Contains fixed-point scale factor

#define adcMAX_ADC_VAL ((uint32_t)4095u) // 12-bit ADC max value

/**
 * @brief Convert raw ADC value of Vin to physical voltage (in mV)
 * @param ulVinRaw Raw ADC code for input voltage
 * @return Physical Vin in millivolts
 */
uint32_t ulVinRawToPhys(const uint32_t ulVinRaw)
{
    uint32_t ulVinPhys;

    // Step 1: Convert ADC code to voltage at ADC pin
    // Step 2: Undo voltage divider using real measured ratio
    // Step 3: Use fixed-point scale factor to avoid floating point
    ulVinPhys = (((ulVinRaw * hwREAL_3V3) / adcMAX_ADC_VAL) * pdSCALE_10K) / hwREAL_VIN_R_RATIO;

    return ulVinPhys;
}

/**
 * @brief Convert raw ADC value of Vout to physical voltage (in mV)
 * @param ulVoutRaw Raw ADC code for output voltage
 * @return Physical Vout in millivolts
 */
uint32_t ulVoutRawToPhys(const uint32_t ulVoutRaw)
{
    uint32_t ulVoutPhys;

    ulVoutPhys = (((ulVoutRaw * hwREAL_3V3) / adcMAX_ADC_VAL) * pdSCALE_10K) / hwREAL_VOUT_R_RATIO;

    return ulVoutPhys;
}
