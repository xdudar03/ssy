/*
 * ili9341.c
 *
 *  Created on: Nov 28, 2019
 *      Author: andrew
 */

// ----------------------------------------------------------------- includes --

#include <stdlib.h> // malloc()
#include <string.h> // memset()
#include <ctype.h>

#include "ili9341.h"
#include "ili9341_gfx.h"
#include "ili9341_font.h"

// ---------------------------------------------------------- private defines --

#define __ILI9341_TOUCH_NORM_SAMPLES__ 8U

// ----------------------------------------------------------- private macros --

#define __IS_SPI_SLAVE(s) (((s) > issNONE) && ((s) < issCOUNT))

#define __SLAVE_SELECT(d, s)  \
  if (__IS_SPI_SLAVE(s)) { ili9341_spi_slave_select((d), (s)); }

#define __SLAVE_RELEASE(d, s) \
  if (__IS_SPI_SLAVE(s)) { ili9341_spi_slave_release((d), (s)); }

// ------------------------------------------------------------ private types --

/* nothing */

// ------------------------------------------------------- exported variables --

/* nothing */

// -------------------------------------------------------- private variables --

/* nothing */

// ---------------------------------------------- private function prototypes --

static void ili9341_reset(ili9341_t *lcd);
static void ili9341_initialize(ili9341_t *lcd);
static ili9341_two_dimension_t ili9341_screen_size(
    ili9341_screen_orientation_t orientation);
static uint8_t ili9341_screen_rotation(
    ili9341_screen_orientation_t orientation);

// ------------------------------------------------------- exported functions --

ili9341_t *ili9341_new(

    SPI_HandleTypeDef *spi_hal,

    GPIO_TypeDef *reset_port,        uint16_t reset_pin,
    GPIO_TypeDef *tft_select_port,   uint16_t tft_select_pin,
    GPIO_TypeDef *data_command_port, uint16_t data_command_pin,

    ili9341_screen_orientation_t orientation,

    GPIO_TypeDef *touch_select_port, uint16_t touch_select_pin,
    GPIO_TypeDef *touch_irq_port,    uint16_t touch_irq_pin,

    ili9341_touch_support_t   touch_support,
    ili9341_touch_normalize_t touch_normalize)
{
  ili9341_t *lcd = NULL;

  if (NULL != spi_hal) {

    if ( (NULL != reset_port)        && IS_GPIO_PIN(reset_pin)         &&
         (NULL != tft_select_port)   && IS_GPIO_PIN(tft_select_pin)    &&
         (NULL != data_command_port) && IS_GPIO_PIN(data_command_pin)  &&
         (orientation > isoNONE)     && (orientation < isoCOUNT)       ) {

      // we must either NOT support the touch interface, OR we must have valid
      // touch interface parameters
      if ( itsSupported != touch_support ||
           ( (NULL != touch_select_port) && IS_GPIO_PIN(touch_select_pin) &&
             (NULL != touch_irq_port)    && IS_GPIO_PIN(touch_irq_pin)    )) {

        if (NULL != (lcd = malloc(sizeof(ili9341_t)))) {

          lcd->spi_hal              = spi_hal;

          lcd->reset_port           = reset_port;
          lcd->reset_pin            = reset_pin;
          lcd->tft_select_port      = tft_select_port;
          lcd->tft_select_pin       = tft_select_pin;
          lcd->data_command_port    = data_command_port;
          lcd->data_command_pin     = data_command_pin;

          lcd->orientation          = orientation;
          lcd->screen_size          = ili9341_screen_size(orientation);

          if (touch_support) {

            lcd->touch_select_port    = touch_select_port;
            lcd->touch_select_pin     = touch_select_pin;
            lcd->touch_irq_port       = touch_irq_port;
            lcd->touch_irq_pin        = touch_irq_pin;

            lcd->touch_support        = touch_support;
            lcd->touch_normalize      = touch_normalize;

          } else {

            lcd->touch_select_port    = NULL;
            lcd->touch_select_pin     = 0;
            lcd->touch_irq_port       = NULL;
            lcd->touch_irq_pin        = 0;

            lcd->touch_support        = touch_support;
            lcd->touch_normalize      = itnNONE;
          }

          ili9341_initialize(lcd);
        }
      }
    }
  }

  return lcd;
}

void ili9341_calibrate_scalar(ili9341_t *lcd,
    uint16_t min_x, uint16_t min_y, uint16_t max_x, uint16_t max_y)
{
  if (NULL == lcd)
    { return; }
}

void ili9341_spi_tft_select(ili9341_t *lcd)
{
  // clear bit indicates the TFT is -active- slave SPI device
  HAL_GPIO_WritePin(lcd->tft_select_port, lcd->tft_select_pin, __GPIO_PIN_CLR__);
}

void ili9341_spi_tft_release(ili9341_t *lcd)
{
  // set bit indicates the TFT is -inactive- slave SPI device
  HAL_GPIO_WritePin(lcd->tft_select_port, lcd->tft_select_pin, __GPIO_PIN_SET__);
}

void ili9341_spi_slave_select(ili9341_t *lcd,
    ili9341_spi_slave_t spi_slave)
{
  switch (spi_slave) {
    case issDisplayTFT:  ili9341_spi_tft_select(lcd);   break;
    default: break;
  }
}

void ili9341_spi_slave_release(ili9341_t *lcd,
    ili9341_spi_slave_t spi_slave)
{
  switch (spi_slave) {

    case issDisplayTFT:  ili9341_spi_tft_release(lcd);   break;
    default: break;
  }
}

void ili9341_spi_write_command(ili9341_t *lcd,
    ili9341_spi_slave_t spi_slave, uint8_t command)
{
  __SLAVE_SELECT(lcd, spi_slave);

  HAL_GPIO_WritePin(lcd->data_command_port, lcd->data_command_pin, __GPIO_PIN_CLR__);
  HAL_SPI_Transmit(lcd->spi_hal, &command, sizeof(command), __SPI_MAX_DELAY__);

  __SLAVE_RELEASE(lcd, spi_slave);
}

void ili9341_spi_write_data(ili9341_t *lcd,
    ili9341_spi_slave_t spi_slave, uint16_t data_sz, uint8_t data[])
{
  __SLAVE_SELECT(lcd, spi_slave);

  HAL_GPIO_WritePin(lcd->data_command_port, lcd->data_command_pin, __GPIO_PIN_SET__);
  HAL_SPI_Transmit(lcd->spi_hal, data, data_sz, __SPI_MAX_DELAY__);

  __SLAVE_RELEASE(lcd, spi_slave);
}

void ili9341_spi_write_data_read(ili9341_t *lcd,
    ili9341_spi_slave_t spi_slave,
    uint16_t data_sz, uint8_t tx_data[], uint8_t rx_data[])
{
  __SLAVE_SELECT(lcd, spi_slave);

  HAL_GPIO_WritePin(lcd->data_command_port, lcd->data_command_pin, __GPIO_PIN_SET__);
  HAL_SPI_TransmitReceive(lcd->spi_hal, tx_data, rx_data, data_sz, __SPI_MAX_DELAY__);

  __SLAVE_RELEASE(lcd, spi_slave);
}

void ili9341_spi_write_command_data(ili9341_t *lcd,
    ili9341_spi_slave_t spi_slave, uint8_t command, uint16_t data_sz, uint8_t data[])
{
  __SLAVE_SELECT(lcd, spi_slave);

  ili9341_spi_write_command(lcd, issNONE, command);
  ili9341_spi_write_data(lcd, issNONE, data_sz, data);

  __SLAVE_RELEASE(lcd, spi_slave);
}

// -------------------------------------------------------- private functions --

static void ili9341_reset(ili9341_t *lcd)
{
  // the reset pin on ILI9341 is active low, so driving low temporarily will
  // reset the device (also resets the touch screen peripheral)
  HAL_GPIO_WritePin(lcd->reset_port, lcd->reset_pin, __GPIO_PIN_CLR__);
  HAL_Delay(200);
  HAL_GPIO_WritePin(lcd->reset_port, lcd->reset_pin, __GPIO_PIN_SET__);

  // ensure both slave lines are open
  ili9341_spi_tft_release(lcd);
}

static void ili9341_initialize(ili9341_t *lcd)
{
  ili9341_reset(lcd);
  ili9341_spi_tft_select(lcd);

  // command list is based on https://github.com/martnak/STM32-ILI9341

  // SOFTWARE RESET
  ili9341_spi_write_command(lcd, issNONE, 0x01);
  HAL_Delay(1000);

  // POWER CONTROL A
  ili9341_spi_write_command_data(lcd, issNONE,
      0xCB, 5, (uint8_t[]){ 0x39, 0x2C, 0x00, 0x34, 0x02 });

  // POWER CONTROL B
  ili9341_spi_write_command_data(lcd, issNONE,
      0xCF, 3, (uint8_t[]){ 0x00, 0xC1, 0x30 });

  // DRIVER TIMING CONTROL A
  ili9341_spi_write_command_data(lcd, issNONE,
      0xE8, 3, (uint8_t[]){ 0x85, 0x00, 0x78 });

  // DRIVER TIMING CONTROL B
  ili9341_spi_write_command_data(lcd, issNONE,
      0xEA, 2, (uint8_t[]){ 0x00, 0x00 });

  // POWER ON SEQUENCE CONTROL
  ili9341_spi_write_command_data(lcd, issNONE,
      0xED, 4, (uint8_t[]){ 0x64, 0x03, 0x12, 0x81 });

  // PUMP RATIO CONTROL
  ili9341_spi_write_command_data(lcd, issNONE,
      0xF7, 1, (uint8_t[]){ 0x20 });

  // POWER CONTROL,VRH[5:0]
  ili9341_spi_write_command_data(lcd, issNONE,
      0xC0, 1, (uint8_t[]){ 0x23 });

  // POWER CONTROL,SAP[2:0];BT[3:0]
  ili9341_spi_write_command_data(lcd, issNONE,
      0xC1, 1, (uint8_t[]){ 0x10 });

  // VCM CONTROL
  ili9341_spi_write_command_data(lcd, issNONE,
      0xC5, 2, (uint8_t[]){ 0x3E, 0x28 });

  // VCM CONTROL 2
  ili9341_spi_write_command_data(lcd, issNONE,
      0xC7, 1, (uint8_t[]){ 0x86 });

  // MEMORY ACCESS CONTROL
  ili9341_spi_write_command_data(lcd, issNONE,
      0x36, 1, (uint8_t[]){ 0x48 });

  // PIXEL FORMAT
  ili9341_spi_write_command_data(lcd, issNONE,
      0x3A, 1, (uint8_t[]){ 0x55 });

  // FRAME RATIO CONTROL, STANDARD RGB COLOR
  ili9341_spi_write_command_data(lcd, issNONE,
      0xB1, 2, (uint8_t[]){ 0x00, 0x18 });

  // DISPLAY FUNCTION CONTROL
  ili9341_spi_write_command_data(lcd, issNONE,
      0xB6, 3, (uint8_t[]){ 0x08, 0x82, 0x27 });

  // 3GAMMA FUNCTION DISABLE
  ili9341_spi_write_command_data(lcd, issNONE,
      0xF2, 1, (uint8_t[]){ 0x00 });

  // GAMMA CURVE SELECTED
  ili9341_spi_write_command_data(lcd, issNONE,
      0x26, 1, (uint8_t[]){ 0x01 });

  // POSITIVE GAMMA CORRECTION
  ili9341_spi_write_command_data(lcd, issNONE,
      0xE0, 15, (uint8_t[]){ 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                             0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 });

  // NEGATIVE GAMMA CORRECTION
  ili9341_spi_write_command_data(lcd, issNONE,
      0xE1, 15, (uint8_t[]){ 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                             0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F });

  // EXIT SLEEP
  ili9341_spi_write_command(lcd, issNONE, 0x11);
  HAL_Delay(120);

  // TURN ON DISPLAY
  ili9341_spi_write_command(lcd, issNONE, 0x29);

  // MADCTL
  ili9341_spi_write_command_data(lcd, issNONE,
      0x36, 1, (uint8_t[]){ ili9341_screen_rotation(lcd->orientation) });

  ili9341_spi_tft_release(lcd);
}

static ili9341_two_dimension_t ili9341_screen_size(
    ili9341_screen_orientation_t orientation)
{
  switch (orientation) {
    default:
    case isoDown:
      return (ili9341_two_dimension_t){ { .width = 240U }, { .height = 320U } };
    case isoRight:
      return (ili9341_two_dimension_t){ { .width = 320U }, { .height = 240U } };
    case isoUp:
      return (ili9341_two_dimension_t){ { .width = 240U }, { .height = 320U } };
    case isoLeft:
      return (ili9341_two_dimension_t){ { .width = 320U }, { .height = 240U } };
  }
}

static uint8_t ili9341_screen_rotation(
    ili9341_screen_orientation_t orientation)
{
  switch (orientation) {
    default:
    case isoDown:
      return 0x40 | 0x08;
    case isoRight:
      return 0x40 | 0x80 | 0x20 | 0x08;
    case isoUp:
      return 0x80 | 0x08;
    case isoLeft:
      return 0x20 | 0x08;
  }
}

ili9341_two_dimension_t ili9341_clip_touch_coordinate(ili9341_two_dimension_t coord,
    ili9341_two_dimension_t min, ili9341_two_dimension_t max)
{
  if (coord.x < min.x) { coord.x = min.x; }
  if (coord.x > max.x) { coord.x = max.x; }
  if (coord.y < min.y) { coord.y = min.y; }
  if (coord.y > max.y) { coord.y = max.y; }

  return coord;
}

