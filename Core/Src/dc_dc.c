#include "main.h"
#include "hw.h"              // Hardware-specific constants and macros (e.g., hwCCMRAM)
#include "dc_dc.h"           // Public DC-DC API
#include "dc_dc_prv.h"       // Private DC-DC helpers, internal types, mode logic, PI clamps
#include <stdbool.h>
#include "pd.h"              // Fixed-point scaling

extern HRTIM_HandleTypeDef hhrtim1;   // High-resolution timer handle (STM32, for PWM/ADC triggers)

//-----------------------------------------------------------------------------
// DUTY CYCLE CONTROL FUNCTIONS
//-----------------------------------------------------------------------------

// Set Buck mode: Buck switch is PWM-controlled, Boost switch is off (0%)
hwCCMRAM
void vSetDutyCycleBuck(const uint32_t ulDA) {
    vSetDutyCycle(ulDA, 0u);
}

// Set Boost mode: Buck switch is always ON (100%), Boost is PWM-controlled
hwCCMRAM
void vSetDutyCycleBoost(const uint32_t ulDB) {
    vSetDutyCycle(10000u, ulDB);
}

// Set Mixed mode: Both Buck and Boost switches are PWM-controlled
hwCCMRAM
void vSetDutyCycleMixed(const uint32_t ulDA, const uint32_t ulDB) {
    vSetDutyCycle(ulDA, ulDB);
}

//-----------------------------------------------------------------------------
// MAIN PI CONTROL LOOP & MODE MANAGEMENT
//-----------------------------------------------------------------------------

/**
 * @brief DC-DC converter PI control loop and mode switching logic
 * @param ulVout        Measured output voltage (mV, from ADC)
 * @param ulVoutTarget  Target (desired) output voltage (mV)
 */
hwCCMRAM
void vDcDcControl(const uint32_t ulVout, const uint32_t ulVoutTarget) {
    // --- PI Controller coefficients and scaling (fixed-point math) ---
    const int32_t lKp = 2000;         // Proportional gain (scaled)
    const int32_t lKi = 2000;         // Integral gain (scaled)
    const int32_t lPiScale = 10000;   // Fixed-point scale (Kp/Ki divided by this)
    const int32_t lPiOutOffs = 5000;  // Output offset (typically halfway point, e.g., 50%)

    // --- Persistent control state (static keeps value between calls) ---
    static int32_t lIntTerm;                    // Integral (I) term accumulator
    static uint32_t ulCntTopHit;                // Count: PI output maxed out (upper limit)
    static uint32_t ulCntBottomHit;             // Count: PI output bottomed out (lower limit)
    static DcDcMode_t xDcDcMode = { .ulMode = dcdcMODE_BUCK }; // Start in Buck mode

    // --- Temporary variables for this cycle ---
    int32_t lError;      // Voltage error (target - measured)
    PiOut_t xPiOut;      // Struct holding PI output value & limit flags

    // --- Compute PI controller error ---
    lError = (int32_t) ulVoutTarget - (int32_t) ulVout;

    // --- Update integral term (with anti-windup) ---
    lIntTerm += lKi * lError / lPiScale;
    lIntTerm = lPiLimitIntTerm(lIntTerm);    // Clamp I-term to prevent windup

    // --- Compute PI output (with offset, for proper duty scaling) ---
    xPiOut.ulD = (uint32_t)(lKp * lError / lPiScale + lIntTerm + lPiOutOffs);

    // --- Clamp PI output to min/max for the current mode, track if at limit ---
    xPiOut = ulPiLimitPiOut(xPiOut.ulD, xDcDcMode.ulMode);

    // --- Track how long the output is stuck at the top/bottom limits ---
    if (true == xPiOut.xTopHit) {
        ulCntTopHit++;
    } else if (0u != ulCntTopHit) {
        ulCntTopHit--;
    }

    if (true == xPiOut.xBottomHit) {
        ulCntBottomHit++;
    } else if (0u != ulCntBottomHit) {
        ulCntBottomHit--;
    }

    // --- Check if mode switch is needed (e.g., can't regulate in current mode) ---
    xDcDcMode = xPiEvalModeSwitch(xDcDcMode.ulMode, ulCntTopHit, ulCntBottomHit);

    // --- If switching modes, re-initialize for the new mode ---
    if (true == xDcDcMode.xSwitch) {
        lIntTerm = 0;           // Reset integral term for stability
        ulCntTopHit = 0u;
        ulCntBottomHit = 0u;

        // Set safe initial duty cycles and ADC timing for the new mode
        if (dcdcMODE_BUCK == xDcDcMode.ulMode) {
            vSetDutyCycleBuck(5000u);           // 50% duty as safe start
            vSetAdcTriggerPoint(2500u);
        }
        else if (dcdcMODE_MIXED == xDcDcMode.ulMode) {
            vSetDutyCycleMixed(8000u, 2000u);   // Initial values for both switches
            vSetAdcTriggerPoint(6000u);
        }
        else { // dcdcMODE_BOOST
            vSetDutyCycleBoost(5000u);          // 50% duty as safe start
            vSetAdcTriggerPoint(2500u);
        }
    }
    // --- If NOT switching modes, update output with current PI calculation ---
    else {
        if (dcdcMODE_BUCK == xDcDcMode.ulMode) {
            vSetDutyCycleBuck(xPiOut.ulD);
            // Adaptive ADC trigger timing for best sampling (centered on ON or OFF)
            if (5000u < xPiOut.ulD) {
                vSetAdcTriggerPoint(xPiOut.ulD / 2u);
            } else {
                vSetAdcTriggerPoint((10000u - xPiOut.ulD) / 2u + xPiOut.ulD);
            }
        }
        else if (dcdcMODE_MIXED == xDcDcMode.ulMode) {
            vSetDutyCycleMixed(8000u, xPiOut.ulD);
            vSetAdcTriggerPoint(6000u);
        }
        else { // dcdcMODE_BOOST
            vSetDutyCycleBoost(xPiOut.ulD);
            if (5000u < xPiOut.ulD) {
                vSetAdcTriggerPoint(xPiOut.ulD / 2u);
            } else {
                vSetAdcTriggerPoint((10000u - xPiOut.ulD) / 2u + xPiOut.ulD);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// ADC TRIGGER CONTROL
//-----------------------------------------------------------------------------

/**
 * @brief Sets ADC trigger point within the PWM cycle (for synchronized sampling)
 * @param ulVal Value from 0 to 10000 (scaled for PWM period)
 */
hwCCMRAM
void vSetAdcTriggerPoint(const uint32_t ulVal) {
    uint32_t ulCmp;

    // Scale the value to the actual PWM timer period
    ulCmp = ulVal * (uint32_t) mainHRTIM_PERIOD / pdSCALE_10K;

    // Set compare value in timer to trigger ADC at the right PWM phase
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_4, ulCmp);
}
