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
#include "adc.h"
#include "i2c.h"
#include "lwip.h"
#include "spi.h"
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

#include "lwip.h"
#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include "mqtt_client.h"
#include "bmp180.h"
#include "ili9341.h"
#include "fonts.h"
#include "st7735.h"


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
extern UART_HandleTypeDef huart3;

PUTCHAR_PROTOTYPE {
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 50);
    return ch;
}

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
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_LWIP_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  mqtt_app_init(&gnetif);
  mqtt_app_start();

  setvbuf(stdout, NULL, _IONBF, 0);

//  dhcp_start(&gnetif); // starts DHCP client
//  for (uint16_t addr = 1; addr < 128; addr++) {
//      if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 2, 100) == HAL_OK) {
//          printf("I2C device found at 0x%02X\r\n", addr);
//      }
//  }
//
//  uint8_t reg = 0xD0;
//  uint8_t chip_id = 0;
//
//  HAL_I2C_Master_Transmit(&hi2c1, BMP180_I2C_ADDR << 1, &reg, 1, 100);
//  HAL_I2C_Master_Receive(&hi2c1, BMP180_I2C_ADDR << 1, &chip_id, 1, 100);
//
//  printf("BMP180 chip id: 0x%02X\r\n", chip_id);

  /* Initializes BMP180 sensor and oversampling settings. */
  BMP180_Init(&hi2c1);
  BMP180_SetOversampling(BMP180_ULTRA);
  /* Update calibration data. Must be called once before entering main loop. */
  BMP180_UpdateCalibrationData();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  printf("ILI9341 init\r\n");

    ILI9341_Init();

    ILI9341_FillScreen(ILI9341_WHITE);

    ILI9341_DrawPixel(0, 0, ILI9341_RED);
    ILI9341_DrawPixel(239, 0, ILI9341_GREEN);
    ILI9341_DrawPixel(0, 319, ILI9341_BLUE);
    ILI9341_DrawPixel(239, 319, ILI9341_BLACK);

    ILI9341_FillRectangle(10, 10, 40, 40, ILI9341_RED);
    ILI9341_FillRectangle(60, 10, 40, 40, ILI9341_GREEN);
    ILI9341_FillRectangle(110, 10, 40, 40, ILI9341_BLUE);

  while (1)
  {
	  MX_LWIP_Process();
	  mqtt_app_process();

	  uint32_t now = HAL_GetTick();

	  static uint32_t last_log = 0;
	  if ((now - last_log) >= 1000U) {
		  last_log = now;
		  printf("IP: %s\r\n", ipaddr_ntoa(&gnetif.ip_addr));
	  }

//	  static uint32_t last_display = 0;
//	  if ((now - last_display) >= 1000U) {
//		  last_display = now;
//
//		  ILI9341_FillRectangle(0, 0, 20, 20, ILI9341_WHITE);
//		  ILI9341_WriteString(10, 10, "Hello STM32", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
//		  ILI9341_WriteString(10, 35, "ILI9341 works", Font_7x10, ILI9341_YELLOW, ILI9341_BLACK);
//		  ILI9341_DrawPixel(10, 10, ILI9341_WHITE);
//	  }

	  if (gnetif.ip_addr.addr != 0) {
//		  printf("IP address is assigned");
		  static uint32_t last_pub = 0;
		  static uint32_t last_ir_log = 0;

		  if (mqtt_app_is_connected()) {
			  uint32_t now = HAL_GetTick();

			  if ((now - last_pub) >= 5000U) {
				  last_pub = now;
				  /* Reads temperature. */

				  int32_t temperature = BMP180_GetTemperature();
				  int32_t pressure = BMP180_GetPressure();

				  char payload[128];
				  int len = snprintf(payload, sizeof(payload),
				                     "{\"temp_c10\":%ld,\"pressure_pa\":%ld}",
				                     (long)temperature,
				                     (long)pressure);

				  if (len > 0 && len < (int)sizeof(payload)) {
				      err_t err = mqtt_app_publish("SSY/bmp180", payload, (u16_t)len);
				      printf("publish err=%d payload=%s\r\n", (int)err, payload);
				  } else {
				      printf("snprintf failed, len=%d\r\n", len);
				  }
			 }
			  if ((now - last_ir_log) >= 500U) {
			      last_ir_log = now;
			      /* IR sensor */
			      HAL_ADC_Start(&hadc1);
			      HAL_ADC_PollForConversion(&hadc1, 100);
			      // PC3
			      uint32_t adc = HAL_ADC_GetValue(&hadc1);
			      HAL_ADC_Stop(&hadc1);
			      // PA6
			      GPIO_PinState state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);

			      char payload[128];
			      int len = snprintf(payload, sizeof(payload),
			    		  	  	  	  "{\"detected\":%d,\"adc\":%lu}",
									  (state == GPIO_PIN_SET) ? 1 : 0, adc);

			      if (len > 0 && len < (int)sizeof(payload)) {
					  err_t err = mqtt_app_publish("SSY/ir", payload, (u16_t)len);
					  printf("publish err=%d payload=%s\r\n", (int)err, payload);
				  } else {
					  printf("snprintf failed, len=%d\r\n", len);
				  }
//			      printf("IR ADC=%lu DOUT=%d\r\n", adc, (state == GPIO_PIN_SET) ? 1 : 0);
			  }
		  }

		  if (mqtt_app_rx_ready()) {
			  printf("RX topic=%s, len=%u\r\n", mqtt_app_rx_topic(), mqtt_app_rx_len());
			  mqtt_app_rx_clear();
		  }
	  }


//	  HAL_Delay(1000);
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
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
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
