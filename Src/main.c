/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "rtc.h"
#include "sdio.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "../../shared/app/blink.h"
#include "../../shared/app/sd.h"
#include "mti.h"

SemaphoreHandle_t xSemaphoreDRDY = NULL;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//extern canInstance_t can1Instance;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//uint32_t can_init();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* TASK BLINK*/
#define APP_BLINK_NAME "BLINK"
#define APP_BLINK_PRIORITY 0
#define APP_BLINK_SIZE 192
StaticTask_t APP_BLINK_BUFFER;
StackType_t APP_BLINK_STACK[ APP_BLINK_SIZE ];

/* TASK SD*/
#define APP_SD_NAME "SD"
#define APP_SD_PRIORITY 0
#define APP_SD_SIZE 1000
StaticTask_t APP_SD_BUFFER;
StackType_t APP_SD_STACK[ APP_SD_SIZE ];

/* TASK MTI*/
#define APP_MTI_NAME "MTI"
#define APP_MTI_PRIORITY 0
#define APP_MTI_SIZE 192
StaticTask_t APP_MTI_BUFFER;
StackType_t APP_MTI_STACK[ APP_MTI_SIZE ];

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_RTC_Init();
  MX_SDIO_SD_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(CAN1_STANDBY_GPIO_Port, CAN1_STANDBY_Pin, GPIO_PIN_RESET);

  config_mti();

  /* FREERTOS TASK CREATION */

  TaskHandle_t xHandle = NULL;

  /* Create the task without using any dynamic memory allocation. */
  xHandle = xTaskCreateStatic(
           task_blink,       /* Function that implements the task. */
           APP_BLINK_NAME,          /* Text name for the task. */
		   APP_BLINK_SIZE,      /* Number of indexes in the xStack array. */
           ( void * ) NULL,    /* Parameter passed into the task. */
           APP_BLINK_PRIORITY,/* Priority at which the task is created. */
		   APP_BLINK_STACK,          /* Array to use as the task's stack. */
           &APP_BLINK_BUFFER );  /* Variable to hold the task's data structure. */

  xHandle = xTaskCreateStatic(
           task_sd,
           APP_SD_NAME,
		   APP_SD_SIZE,
           ( void * ) NULL,
           APP_SD_PRIORITY,
		   APP_SD_STACK,
           &APP_SD_BUFFER );

  xHandle = xTaskCreateStatic(
           task_mti,
           APP_MTI_NAME,
		   APP_MTI_SIZE,
           ( void * ) NULL,
           APP_MTI_PRIORITY,
		   APP_MTI_STACK,
           &APP_MTI_BUFFER );

	/* Start the scheduler. */
	vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == sd_detect_Pin) {
      //app_sd_detect_handler();
    }else if(GPIO_Pin == MTI_DRDY_Pin){

    	xSemaphoreGiveFromISR( xSemaphoreDRDY, NULL );

    }
}

void EXTI4_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(MTI_DRDY_Pin);
}

/*uint32_t can_init()
{
    can1Instance.instance = CAN1;
    can1Instance.debugFreeze = 0;
    can1Instance.opMode = normal;
    can1Instance.baudPrescaler = 3;
    can1Instance.timeQuanta1 = 14;
    can1Instance.timeQuanta2 = 11;
    can1Instance.timeReSync = 2;

    canInit(&can1Instance);
    //init interruption for fan 1 fifo 0
    can1Fifo0InitIt(&can1Instance);
    can1Fifo0RegisterCallback(can_regUpdateCallback);

    //init filters for boards
    canFilter_t filter = {0};

    filter.mask11.mask0 = BOARD_ID_MASK;
    filter.mask11.ID0 = BOARD_EMERGENCY_ID_SHIFTED;
    filter.mask11.mask1 = BOARD_ID_MASK;
    filter.mask11.ID1 = BOARD_MISSION_ID_SHIFTED;
    canSetFilter(&can1Instance, &filter,mask11Bit, 0, 0);

    filter.mask11.mask0 = BOARD_ID_MASK;
    filter.mask11.ID0 = BOARD_COMMUNICATION_ID_SHIFTED;
    filter.mask11.mask1 = BOARD_ID_MASK;
    filter.mask11.ID1 = BOARD_COMMUNICATION_ID_SHIFTED;
    canSetFilter(&can1Instance, &filter,mask11Bit, 1, 0);

    NVIC_SetPriority(20, 10);
    return 0;
}*/
/* USER CODE END 4 */

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

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
