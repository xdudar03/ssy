/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "eth.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
//uint8_t rx_char;
uint32_t led_ticks = 1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM6_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  CLI_UART_Init();
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

  // RED
//  LED_init(GPIOB, 14);
//
//  for(int i = 0; i < 3; i++)
//  {
//      LED_ON(GPIOB, 14);
//      HAL_Delay(500);
//      LED_OFF(GPIOB, 14);
//      HAL_Delay(500);
//  }
//
//  //BLUE
//  LED_init(GPIOB, 7);
//
//	for(int i = 0; i < 3; i++)
//	{
//		LED_ON(GPIOB, 7);
//		HAL_Delay(500);
//		LED_OFF(GPIOB, 7);
//		HAL_Delay(500);
//	}
//
//  //GREEN
//  LED_init(GPIOB, 0);
//
//	for(int i = 0; i < 3; i++)
//	{
//		LED_ON(GPIOB, 0);
//		HAL_Delay(500);
//		LED_OFF(GPIOB, 0);
//		HAL_Delay(500);
//	}
//
//	USER_button(GPIOC, 13);
//	LED_init(GPIOB, 7);
//
//    // Keep track of whether the button is pressed.
//    uint8_t button_down = 0;
//    while (1) {
//      if (!(GPIOC->IDR & (1U << 13))) {
//        // The button is pressed; if it was not already
//        // pressed, change the LED state.
//        HAL_Delay(20);
//        if (!button_down) {
//          GPIOB->ODR ^= (1U << 7);
//        }
//        button_down = 1;
//      }
//      else {
//        button_down = 0;
//      }
//    }

// ukol 1
//  char *msg = "Hello from STM32!\r\n";
//  HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
//
//  HAL_UART_Receive_IT(&huart3, &rx_char, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

//	  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)== GPIO_PIN_RESET){
//		  HAL_Delay(20);
//		  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)== GPIO_PIN_RESET){
//			  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
//			  while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)== GPIO_PIN_RESET);
//		  }
//	  }
// 		ukol 1
//	  if(HAL_UART_Receive(&huart3, &rx_char, 1, HAL_MAX_DELAY) == HAL_OK) {
//		  HAL_UART_Transmit(&huart3, &rx_char, 1, HAL_MAX_DELAY);
//	  }
	  CLI_UART_Process();


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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
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
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13)
    {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart -> Instance == USART3) {
		CLI_UART_RxCallback();
	}
}

void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM6) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
	}
	if (htim->Instance == TIM4) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
	}
	if (htim->Instance == TIM1) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
        uint32_t compare = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2);
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, compare + led_ticks);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
