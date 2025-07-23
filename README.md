# STM32-Buck-Boost-DC-DC-Converter-Controller
This project implements a firmware-controlled DC-DC power converter that operates in buck, boost, or mixed mode using an STM32 microcontroller. It leverages HRTIM for high-resolution PWM generation, ADC for voltage sensing, and FreeRTOS for task scheduling.

## Features

- Voltage measurement via STM32 ADC in injected mode
- High-resolution PWM generation using HRTIM Timer A and B
- Buck, Boost, and Mixed mode operation 
- PI control loop for regulating output voltage

## Hardware Requirements

- STM32 microcontroller with HRTIM peripheral (e.g., STM32F334 series)
- External DC-DC converter circuitry (MOSFETs, inductor, capacitors)
- Voltage divider circuits for measuring Vin and Vout
- USB or ST-Link programmer for flashing the firmware

## File Structure and Explanation

### `Inc/` – Header Files (Interfaces and Definitions)

| File            | Description |
|-----------------|-------------|
| `adc.h`         | Declares functions to convert raw ADC values (`Vin`, `Vout`) to physical voltages using scaling and divider ratios. |
| `dc_dc.h`       | Public interface for the main control logic managing duty cycles and converter mode. |
| `dc_dc_prv.h`   | Internal helper macros, limits, and type definitions used for converter logic, duty cycle evaluation, and PI limits. |
| `hw.h`          | Defines hardware-related constants such as voltage divider ratios, ADC calibration values, and PWM period. |
| `pd.h`          | Provides a scaling factor (`pdSCALE_10K`) used to avoid floating-point math by working with integers. |
| `timer.h`       | Declares functions related to runtime statistics |

### `Src/` – Source Files (Implementation)

| File            | Description |
|-----------------|-------------|
| `adc.c`         | Implements voltage conversion logic. Converts ADC readings to physical voltages using calibration and divider ratios. |
| `dc_dc.c`       | Main control logic. Interfaces with `dc_dc_prv.c`. |
| `dc_dc_prv.c`   | Handles PWM duty cycle generation (`vSetDutyCycle`), mode switching (Buck ↔ Mixed ↔ Boost), and PI limiting. |
| `timer.c`       | Provides a simple tick counter for runtime measurements. Used with FreeRTOS for profiling. |

## How the System Works

1. **Voltage Sensing:**  
   The STM32 ADC measures `Vin` and `Vout` through voltage dividers. The raw ADC values are converted to physical voltages using `ulVinRawToPhys()` and `ulVoutRawToPhys()`.

2. **PWM Generation:**  
   HRTIM Timer A and B generate high-resolution PWM signals to control MOSFETs for Buck or Boost behavior. The duty cycles are managed by `vSetDutyCycle()`.

3. **Mode Switching:**  
   The converter switches between Buck, Boost, or Mixed mode automatically based on feedback conditions. Logic is implemented in `xPiEvalModeSwitch()`.

4. **PI Controller (optional):**  
   A PI control loop (if enabled in `dc_dc.c`) adjusts duty cycles to maintain a stable output voltage. Integral term limits and output clamping are handled in `lPiLimitIntTerm()` and `ulPiLimitPiOut()`.

5. **RTOS Tasks:**  
   - `vComTask()` handles communication (placeholder).
   - `vDcDcTask()` handles ADC sampling, conversion, and control logic invocation.

## How to Build and Flash

1. Open the project in **STM32CubeIDE**.
2. Connect your STM32 board via USB or ST-Link.
3. Build the project.
4. Flash to the target device.

## Notes

- The code is written to be portable across STM32 MCUs with HRTIM and ADC injected trigger support.
- Calibration and tuning may be required based on your actual circuit, especially for resistor values in voltage dividers and dead times in HRTIM.
