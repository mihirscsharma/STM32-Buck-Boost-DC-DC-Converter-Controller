#include "main.h"
#include "hw.h"
#include "dc_dc_prv.h"
#include <stdbool.h>
#include "pd.h"

extern HRTIM_HandleTypeDef hhrtim1; // High-resolution timer for PWM

//------------------------------------------------------------------------------
// Set hardware PWM duty cycles for buck and boost
// ulDA: buck duty (0–10000 = 0–100%), ulDB: boost duty (0–10000 = 0–100%)
hwCCMRAM
void vSetDutyCycle(const uint32_t ulDA, const uint32_t ulDB)
{
    uint32_t ulCmpA; // Compare value for buck switch
    uint32_t ulCmpB; // Compare value for boost switch

    // Boost channel: if duty is 0% (OFF), set compare to period (no pulse)
    if (0u == ulDB) {
        ulCmpB = (uint32_t) mainHRTIM_PERIOD;
    } else {
        // Scale duty (0–10000) to timer period
        ulCmpB = ulDB * (uint32_t) mainHRTIM_PERIOD / pdSCALE_10K;
    }

    // Buck channel: if duty is 100% (ON), set compare to period+1 (never triggers, always ON)
    if (10000u == ulDA) {
        ulCmpA = (uint32_t) mainHRTIM_PERIOD + 1u;
    } else {
        ulCmpA = ulDA * (uint32_t) mainHRTIM_PERIOD / pdSCALE_10K;
    }

    // Write compare values to PWM hardware registers (STM32 HRTIM)
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, ulCmpA); // Buck
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_2, ulCmpB); // Boost
}

//------------------------------------------------------------------------------
// Get current buck/boost duty cycles as 0–10000 scaled values
hwCCMRAM
DutyCycle_t xGetDutyCycle(void)
{
    uint32_t ulCmpA = __HAL_HRTIM_GETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1);
    uint32_t ulCmpB = __HAL_HRTIM_GETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_2);
    DutyCycle_t xDutyCycle;

    // Boost: if compare == period, duty is 0%
    if ((uint32_t) mainHRTIM_PERIOD == ulCmpB) {
        xDutyCycle.ulDB = 0u;
    } else {
        // Scale compare value back to 0–10000
        xDutyCycle.ulDB = ulCmpB * pdSCALE_10K / (uint32_t) mainHRTIM_PERIOD;
    }

    // Buck: if compare == period+1, duty is 100%
    if (((uint32_t) mainHRTIM_PERIOD + 1u) == ulCmpA) {
        xDutyCycle.ulDA = 10000u;
    } else {
        xDutyCycle.ulDA = ulCmpA * pdSCALE_10K / (uint32_t) mainHRTIM_PERIOD;
    }

    return xDutyCycle;
}

//------------------------------------------------------------------------------
// Decide if a mode switch should happen based on PI controller hitting its limits
// ulModeOld: current mode, ulCntTop: # of cycles output has hit max, ulCntBottom: # at min
hwCCMRAM
DcDcMode_t xPiEvalModeSwitch(const uint32_t ulModeOld, const uint32_t ulCntTop, const uint32_t ulCntBottom)
{
    DcDcMode_t xDcDcMode = { .ulMode = ulModeOld, .xSwitch = false };

    // If in buck mode, and output hit top limit for dcdcMAX_CNT cycles, switch to mixed mode
    if (dcdcMODE_BUCK == ulModeOld) {
        if (dcdcMAX_CNT == ulCntTop) {
            xDcDcMode.ulMode = dcdcMODE_MIXED;
            xDcDcMode.xSwitch = true;
        }
    }
    // If in mixed mode, top limit = go to boost; bottom limit = go to buck
    else if (dcdcMODE_MIXED == ulModeOld) {
        if (dcdcMAX_CNT == ulCntTop) {
            xDcDcMode.ulMode = dcdcMODE_BOOST;
            xDcDcMode.xSwitch = true;
        } else if (dcdcMAX_CNT == ulCntBottom) {
            xDcDcMode.ulMode = dcdcMODE_BUCK;
            xDcDcMode.xSwitch = true;
        }
    }
    // If in boost mode and output hit bottom for dcdcMAX_CNT cycles, switch to mixed
    else { // BOOST
        if (dcdcMAX_CNT == ulCntBottom) {
            xDcDcMode.ulMode = dcdcMODE_MIXED;
            xDcDcMode.xSwitch = true;
        }
    }
    return xDcDcMode;
}

//------------------------------------------------------------------------------
// Limit the integral term (I-term) of the PI controller to prevent runaway
hwCCMRAM
int32_t lPiLimitIntTerm(const int32_t lIntTermOld)
{
    int32_t lIntTerm = lIntTermOld;
    if (dcdcINT_TERM_LIMIT < lIntTerm) {
        lIntTerm = dcdcINT_TERM_LIMIT;
    } else if (-dcdcINT_TERM_LIMIT > lIntTerm) {
        lIntTerm = -dcdcINT_TERM_LIMIT;
    }
    return lIntTerm;
}

//------------------------------------------------------------------------------
// Limit the PI controller's output and set flags if limits were hit
// Returns: PiOut_t struct (ulD = limited value, xTopHit/xBottomHit = flags)
hwCCMRAM
PiOut_t ulPiLimitPiOut(const uint32_t ulPiOutDOld, const uint32_t ulMode)
{
    PiOut_t xPiOut = { .ulD = ulPiOutDOld, .xTopHit = false, .xBottomHit = false };

    if (dcdcMODE_BUCK == ulMode) {
        if (dcdcMAX_D_A <= ulPiOutDOld) {
            xPiOut.ulD = dcdcMAX_D_A;      // Clamp to max for buck
            xPiOut.xTopHit = true;
        } else if (dcdcMIN_D_A >= ulPiOutDOld) {
            xPiOut.ulD = dcdcMIN_D_A;      // Clamp to min for buck
            xPiOut.xBottomHit = true;
        }
    } else if (dcdcMODE_MIXED == ulMode) {
        if (dcdcMAX_D_B_MIXED <= ulPiOutDOld) {
            xPiOut.ulD = dcdcMAX_D_B_MIXED; // Clamp to max for mixed
            xPiOut.xTopHit = true;
        } else if (dcdcMIN_D_B_MIXED >= ulPiOutDOld) {
            xPiOut.ulD = dcdcMIN_D_B_MIXED; // Clamp to min for mixed
            xPiOut.xBottomHit = true;
        }
    } else { // BOOST
        if (dcdcMAX_D_B <= ulPiOutDOld) {
            xPiOut.ulD = dcdcMAX_D_B;       // Clamp to max for boost
            xPiOut.xTopHit = true;
        } else if (dcdcMIN_D_B >= ulPiOutDOld) {
            xPiOut.ulD = dcdcMIN_D_B;       // Clamp to min for boost
            xPiOut.xBottomHit = true;
        }
    }
    return xPiOut;
}
