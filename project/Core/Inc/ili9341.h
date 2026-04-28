#ifndef __ILI9341_H
#define __ILI9341_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "main.h"
#include "spi.h"
#include "gpio.h"

#define __GPIO_PIN_CLR__ GPIO_PIN_RESET
#define __GPIO_PIN_SET__ GPIO_PIN_SET
#define __SPI_MAX_DELAY__ HAL_MAX_DELAY
#define __SPI_TX_BLOCK_MAX__ (1U * 1024U)

#define __MSBYTEu16(u) (uint8_t)(((uint16_t)(u) >> 8U) & 0xFF)
#define __LSBYTEu16(u) (uint8_t)(((uint16_t)(u)) & 0xFF)
#define ibOK(v)   ((v) == ibTrue)
#define ibNOT(v)  ((v) != ibTrue)

static inline uint16_t __LEu16(const uint16_t *v) {
    uint16_t x = *v;
    return (uint16_t)((x >> 8) | (x << 8));
}

typedef struct ili9341 ili9341_t;

typedef enum {
  ibFalse = 0,
  ibNo = ibFalse,
  ibTrue = 1,
  ibYes = ibTrue,
} ili9341_bool_t;

typedef struct {
  union { uint16_t x; uint16_t width; };
  union { uint16_t y; uint16_t height; };
} ili9341_two_dimension_t;

typedef enum {
  isoNONE = -1,
  isoDown,
  isoPortrait = isoDown,
  isoRight,
  isoLandscape = isoRight,
  isoUp,
  isoPortraitFlip = isoUp,
  isoLeft,
  isoLandscapeFlip = isoLeft,
  isoCOUNT
} ili9341_screen_orientation_t;

typedef enum {
  itsNONE = -1,
  itsNotSupported,
  itsSupported,
  itsCOUNT
} ili9341_touch_support_t;

typedef enum {
  itnNONE = -1,
  itnNotNormalized,
  itnNormalized,
  itnCOUNT
} ili9341_touch_normalize_t;

typedef enum {
  issNONE = -1,
  issDisplayTFT,
  issTouchScreen,
  issCOUNT
} ili9341_spi_slave_t;

typedef HAL_StatusTypeDef ili9341_status_t;

struct ili9341 {
  SPI_HandleTypeDef *spi_hal;
  GPIO_TypeDef *reset_port;
  uint16_t reset_pin;
  GPIO_TypeDef *tft_select_port;
  uint16_t tft_select_pin;
  GPIO_TypeDef *data_command_port;
  uint16_t data_command_pin;
  ili9341_screen_orientation_t orientation;
  ili9341_two_dimension_t screen_size;

  GPIO_TypeDef *touch_select_port;
  uint16_t touch_select_pin;
  GPIO_TypeDef *touch_irq_port;
  uint16_t touch_irq_pin;
  ili9341_touch_support_t touch_support;
  ili9341_touch_normalize_t touch_normalize;
};

ili9341_t *ili9341_new(
    SPI_HandleTypeDef *spi_hal,
    GPIO_TypeDef *reset_port,
    uint16_t reset_pin,
    GPIO_TypeDef *tft_select_port,
    uint16_t tft_select_pin,
    GPIO_TypeDef *data_command_port,
    uint16_t data_command_pin,
    ili9341_screen_orientation_t orientation,
    GPIO_TypeDef *touch_select_port,
    uint16_t touch_select_pin,
    GPIO_TypeDef *touch_irq_port,
    uint16_t touch_irq_pin,
    ili9341_touch_support_t touch_support,
    ili9341_touch_normalize_t touch_normalize);

void ili9341_spi_tft_select(ili9341_t *lcd);
void ili9341_spi_tft_release(ili9341_t *lcd);
void ili9341_spi_write_command(ili9341_t *lcd, ili9341_spi_slave_t spi_slave, uint8_t command);
void ili9341_spi_write_data(ili9341_t *lcd, ili9341_spi_slave_t spi_slave, uint16_t data_sz, uint8_t data[]);
void ili9341_spi_write_command_data(ili9341_t *lcd, ili9341_spi_slave_t spi_slave, uint8_t command, uint16_t data_sz, uint8_t data[]);

#ifdef __cplusplus
}
#endif

#endif
