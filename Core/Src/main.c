/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

// USER: Added custom headers for DC-DC control and ADC processing
#include "dc_dc.h"
#include "adc.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER may define custom types here (currently empty) */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER may define custom #defines here (currently empty) */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER may define custom macros here (currently empty) */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
HRTIM_HandleTypeDef hhrtim1;

osThreadId xComTaskHandle;
osThreadId xDcDcTaskHandle;

/* USER CODE BEGIN PV */

// USER: Added custom static variables to store ADC results (raw and physical units)
static uint32_t ulVinRaw;
static uint32_t ulVoutRaw;
static uint32_t ulVinPhys;
static uint32_t ulVoutPhys;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_HRTIM1_Init(void);
static void MX_ADC1_Init(void);
void vComTask(void const *argument);
void vDcDcTask(void const *argument);

/* USER CODE BEGIN PFP */
/* USER may declare private function prototypes here (currently empty) */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER may add initialization code here (currently empty) */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */
    // USER: Early initialization if needed (empty)
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/
    HAL_Init();

    /* USER CODE BEGIN Init */
    // USER: Post-HAL initialization code (empty)
    /* USER CODE END Init */

    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    // USER: System initialization code (empty)
    /* USER CODE END SysInit */

    MX_GPIO_Init();
    MX_HRTIM1_Init();
    MX_ADC1_Init();

    /* USER CODE BEGIN 2 */
    // USER: Code runs after all peripherals are initialized (empty)
    /* USER CODE END 2 */

    /* USER CODE BEGIN RTOS_MUTEX */
    // USER: Mutex initialization (empty)
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    // USER: Semaphore initialization (empty)
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    // USER: Timer initialization (empty)
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    // USER: Queue initialization (empty)
    /* USER CODE END RTOS_QUEUES */

    // FreeRTOS thread definitions and creations
    osThreadDef(xComTask, vComTask, osPriorityNormal, 0, 256);
    xComTaskHandle = osThreadCreate(osThread(xComTask), NULL);

    osThreadDef(xDcDcTask, vDcDcTask, osPriorityNormal, 0, 256);
    xDcDcTaskHandle = osThreadCreate(osThread(xDcDcTask), NULL);

    /* USER CODE BEGIN RTOS_THREADS */
    // USER: Create additional RTOS threads (empty)
    /* USER CODE END RTOS_THREADS */

    osKernelStart();

    // The scheduler takes control, but if it ever returns, loop forever
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
        // USER: Code for main loop (if not using RTOS)
        /* USER CODE END 3 */
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    // Standard CubeMX code for setting up the MCU's clocks
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_HRTIM1 | RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
    PeriphClkInit.Hrtim1ClockSelection = RCC_HRTIM1CLK_PLLCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{
    /* USER CODE BEGIN ADC1_Init 0 */
    // USER: Code before ADC init (empty)
    /* USER CODE END ADC1_Init 0 */

    ADC_MultiModeTypeDef multimode = { 0 };
    ADC_InjectionConfTypeDef sConfigInjected = { 0 };

    /* USER CODE BEGIN ADC1_Init 1 */
    // USER: Code after ADC structures (empty)
    /* USER CODE END ADC1_Init 1 */

    // CubeMX generated ADC configuration code...

    /* USER CODE BEGIN ADC1_Init 2 */
    // USER: Code after ADC init (empty)
    /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief HRTIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_HRTIM1_Init(void)
{
    /* USER CODE BEGIN HRTIM1_Init 0 */
    // USER: Code before HRTIM init (empty)
    /* USER CODE END HRTIM1_Init 0 */

    // ... CubeMX generated code for high-res timer setup

    /* USER CODE BEGIN HRTIM1_Init 2 */
    // USER: Code after HRTIM init (empty)
    /* USER CODE END HRTIM1_Init 2 */
    HAL_HRTIM_MspPostInit(&hhrtim1);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    // CubeMX generated code for GPIO port clock enables
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

/* USER CODE BEGIN 4 */
// USER: Other private user functions (currently empty)
/* USER CODE END 4 */

/* USER CODE BEGIN Header_vComTask */
/**
 * @brief  Function implementing the xComTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_vComTask */
void vComTask(void const *argument)
{
    /* USER CODE BEGIN 5 */
    // USER: Main logic for communication thread (currently empty)
    uint32_t ulThreadNotification;

    for (;;)
    {
        ulThreadNotification = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (ulThreadNotification)
        {
            // USER: Handle thread notification event (currently empty)
        }
    }
    /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_vDcDcTask */
/**
 * @brief Function implementing the xDcDcTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_vDcDcTask */
void vDcDcTask(void const *argument)
{
    /* USER CODE BEGIN vDcDcTask */
    // USER: Main logic for DC-DC control thread

    uint32_t ulThreadNotification;

    // --- USER: Initialize DC-DC converter to mid duty cycle and set ADC trigger point
    vSetDutyCycleBuck(5000u);
    vSetAdcTriggerPoint(2500u);

    // --- USER: Start HRTIM waveform outputs and counters
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
    HAL_HRTIM_WaveformCountStart_IT(&hhrtim1, HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B);

    // --- USER: Calibrate and start ADC in injected mode with interrupts
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_InjectedStart_IT(&hadc1);

    // --- USER: Main RTOS task loop
    for (;;) //infinite loop
    {
        ulThreadNotification = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (ulThreadNotification)
        {
            // --- USER: Get ADC injected values (input/output voltage raw)
            ulVinRaw = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
            ulVoutRaw = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);

            // --- USER: Convert raw ADC values to physical voltages
            ulVinPhys = ulVinRawToPhys(ulVinRaw);
            ulVoutPhys = ulVoutRawToPhys(ulVoutRaw);

            //  Run closed-loop DC-DC control
             vDcDcControl(ulVoutPhys, 5000u);
        }
    }
    /* USER CODE END vDcDcTask */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 0 */
    // USER: Additional code before tick increment (empty)
    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM1)
    {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */
    // USER: Additional code after tick increment (empty)
    /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    // USER: Custom error handler (could add logging, blinking LED, etc.)
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  // USER: Custom assert failure logic (print error, etc.)
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
