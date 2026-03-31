/*
 * debug_uart.c
 *
 *  Created on: 10. 2. 2026
 *      Author: Student
 */

#include "putchar.h"

extern UART_HandleTypeDef huart3;

PUTCHAR_PROTOTYPE
{
  /* Odeslání jednoho znaku přes USART3 sdefinovaným timeoutem */
  HAL_UART_Transmit(&huart3, (uint8_t * )&ch, 1, 50);
  return ch;
}
