/*
 * cv2.c
 *
 *  Created on: 24. 2. 2026
 *      Author: Student
 */

#include <stdint.h>
#include "stm32f4xx_hal.h"

void LED_init(GPIO_TypeDef *port, uint16_t pin)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    port->MODER &= ~(3U << (pin * 2));
    port->MODER |=  (1U << (pin * 2));
}

void LED_ON(GPIO_TypeDef *port, uint16_t pin)
{
    port->BSRR = (1U << pin);
}

void LED_OFF(GPIO_TypeDef *port, uint16_t pin)
{
    port->BSRR = (1U << (pin + 16));
}

void USER_button(GPIO_TypeDef *button_port, uint16_t button_pin) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	button_port->MODER  &= ~(3U << (button_pin*2));
	button_port->PUPDR  &= ~(3U << (button_pin*2));
	button_port->PUPDR  |=  (3U << (button_pin*2));
}
