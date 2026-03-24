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
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "putchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ODECET
#define KONSTANTA 2

#define UPPER_CASE 1
#define NORMAL_CASE 2
//#define DIRECTION_UP
#define DIRECTION_DOWN
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

int a = 10;
char field[52];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

int generateField(int type) {
    int i = 0;

    if (type != UPPER_CASE && type != NORMAL_CASE) {
        printf("PROGRAM ERROR\n");
        return 0;
    }

    if (type == NORMAL_CASE) {
        for (char c = 'a'; c <= 'z'; c++)
            field[i++] = c;
        for (char c = 'A'; c <= 'Z'; c++)
            field[i++] = c;
    } else {
        for (char c = 'A'; c <= 'Z'; c++)
            field[i++] = c;
        for (char c = 'a'; c <= 'z'; c++)
            field[i++] = c;
    }

    return 1;
}

int capsLetters(int type) {
    if (type != UPPER_CASE && type != NORMAL_CASE) {
        printf("PROGRAM ERROR\n");
        return 0;
    }

    for (int i = 0; i < 52; i++) {
        if (type == UPPER_CASE)
            field[i] = toupper(field[i]);
        else
            field[i] = tolower(field[i]);
    }

    return 1;
}

void printField(void) {
#ifdef DIRECTION_UP
	for (int i = 0; i < 52; i++)
	    printf("%c ", field[i]);
#endif
#ifdef DIRECTION_DOWN
	for (int i = 51; i >= 0; i--)
		printf("%c ", field[i]);
#endif
}
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
  // ukol 3
  #ifdef ODECET
   a=a-KONSTANTA;
  #endif
   printf("Ukol 3 vysledek: %d ", a);

   // ukol 4
   unsigned char b = 255;
   unsigned char c = 255;
   int d;
   d = (int)b + (int)c;
   printf("Ukol 4 vysledek: %d ", d);

   //ukol 5
   int f = 24;
   f = f >> 3;
   f = f - 1;
   f = f&2;
   printf("Ukol 5 vysledek: %d ", f);

   // ukol 6
   int e = 200;
   char text[15];
   sprintf(text,"Hodnota=%d ", e);
   printf("%s", text);

   // ukol 7
   generateField(UPPER_CASE);

   capsLetters(NORMAL_CASE);

   printField();
   // ukol 8
   int promenna = 23;
   int *pointer = &promenna;

   printf("Hodnota: %d ", promenna);
   printf("Ukazatel: %d ", *pointer);

   printf("Pamet: %p ", (void *)&promenna);
   printf("Pamet ukazalete: %p ", (void *)pointer);


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
  /* USER CODE BEGIN 2 */

  // ukol 2
  setvbuf(stdout, NULL, _IONBF, 0);
  printf("Hello world!\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //ukol 1
	  HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin); // Přepne stav zelené LED
	  HAL_Delay(500); // Počká 500 milisekund
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
