/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 Perry Hung.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/**
 * @file HardwareSerial.cpp
 * @brief Wirish serial port implementation.
 */

#include "libmaple.h"
#include "gpio.h"
#include "timer.h"

#include "HardwareSerial.h"
#include "boards.h"
#include "adafruit_featherlib.h"

#define TX1 BOARD_USART1_TX_PIN
#define RX1 BOARD_USART1_RX_PIN

#define TX2 BOARD_USART2_TX_PIN
#define RX2 BOARD_USART2_RX_PIN

#define TX4 BOARD_UART4_TX_PIN
#define RX4 BOARD_UART4_RX_PIN

HardwareSerial Serial1(USART1, TX1, RX1);
HardwareSerial Serial2(USART2, TX2, RX2);
HardwareSerial Serial4(UART4 , TX4, RX4);

HardwareSerial::HardwareSerial(usart_dev *usart_device,
                               uint8 tx_pin,
                               uint8 rx_pin) {
    this->usart_device = usart_device;
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
    this->isConnected = false;
}

/*
 * Set up/tear down
 */

void HardwareSerial::begin(uint32 baud) {
    ASSERT(baud <= usart_device->max_baud);

    if (baud > usart_device->max_baud) {
        return;
    }

    const stm32_pin_info *txi = &PIN_MAP[tx_pin];
    const stm32_pin_info *rxi = &PIN_MAP[rx_pin];

    gpio_set_mode(txi->gpio_device, txi->gpio_bit, (gpio_pin_mode)(GPIO_AF_OUTPUT_PP | GPIO_PUPD_INPUT_PU | 0x700));
    gpio_set_mode(rxi->gpio_device, rxi->gpio_bit, (gpio_pin_mode)(GPIO_MODE_AF      | GPIO_PUPD_INPUT_PU | 0x700));


    int afnum = (usart_device == UART4 ) ? 8 : 7;

    gpio_set_af_mode(txi->gpio_device, txi->gpio_bit, afnum);
    gpio_set_af_mode(rxi->gpio_device, rxi->gpio_bit, afnum);

    usart_init(usart_device);
    usart_set_baud_rate(usart_device, baud);
    usart_enable(usart_device);
}

void HardwareSerial::end(void) {
    usart_disable(usart_device);
}

/*
 * I/O
 */

int HardwareSerial::read(void) {
	if(usart_data_available(usart_device) > 0) {
		return usart_getc(usart_device);
	} else {
		return -1;
	}
}

int HardwareSerial::available(void) {
    return usart_data_available(usart_device);
}

int HardwareSerial::peek(void)
{
    return usart_peek(usart_device);
}
    
uint32 HardwareSerial::pending(void) {
    return usart_data_pending(usart_device);
}

size_t HardwareSerial::write(uint8_t ch) {
  usart_putc(usart_device, ch);
  return 1;
}

void HardwareSerial::flush(void) {
    usart_reset_rx(usart_device);
}

bool HardwareSerial::enableFlowControl(bool use_cts, bool use_rts)
{
  // only UART2 has flowcontrol supports
  if ( this->usart_device != USART2 ) return false;

  if ( use_cts )
  {
    const stm32_pin_info *cts = &PIN_MAP[BOARD_USART2_CTS_PIN];
    gpio_set_mode(cts->gpio_device, cts->gpio_bit, (gpio_pin_mode) GPIO_AF_OUTPUT_PP);
    gpio_set_af_mode(cts->gpio_device, cts->gpio_bit, 7);
  }

  if ( use_rts )
  {
    const stm32_pin_info *rts = &PIN_MAP[BOARD_USART2_RTS_PIN];
    gpio_set_mode(rts->gpio_device, rts->gpio_bit, (gpio_pin_mode) GPIO_AF_OUTPUT_PP);
    gpio_set_af_mode(rts->gpio_device, rts->gpio_bit, 7);
  }

  usart_enable_flowcontrol(this->usart_device, use_cts, use_rts);

  return true;
}
