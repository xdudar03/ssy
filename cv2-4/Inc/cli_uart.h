/*
 * cli_uart.h
 *
 *  Created on: 3. 3. 2026
 *      Author: Student
 */

#ifndef INC_CLI_UART_H_
#define INC_CLI_UART_H_

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "main.h"
#include "usart.h"
#include "cv2.h"
#include "tim.h"

void CLI_UART_Print(const char *s);
void CLI_UART_Printf(const char *fmt, ...);
void CLI_UART_Init();
void CLI_UART_OnRxByte(uint8_t b);
void CLI_UART_Process(void);
void CLI_UART_RxCallback(void);


#endif /* INC_CLI_UART_H_ */
