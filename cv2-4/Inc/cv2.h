/*
 * cv2.h
 *
 *  Created on: 24. 2. 2026
 *      Author: Student
 */

#ifndef INC_CV2_H_
#define INC_CV2_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

void LED_init(GPIO_TypeDef *port, uint16_t pin);
void LED_ON(GPIO_TypeDef *port, uint16_t pin);
void LED_OFF(GPIO_TypeDef *port, uint16_t pin);
void USER_button(GPIO_TypeDef *button_port, uint16_t button_pin);

#endif /* INC_CV2_H_ */
