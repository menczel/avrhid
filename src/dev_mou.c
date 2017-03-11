/*
  hid_mouse.c
  Copyright (c) 2011, Peter Barrett
  Modifications: Copyright (C) 2017 Laszlo Menczel

  This is free software distributed under the terms of the MIT license
  (see the file LICENSE or https://opensource.org/licenses/MIT).
 
  ======================================================================

  This is a simple program to test the HID mouse code. If it works
  then it should be replaced by the real device handler that polls the
  I/O ports, and updates the state structure of the mouse and sends the
  mouse report.

  Moves the mouse cursor in a small circle every 5 seconds. The LED is
  ON while the cursor moves, otherwise it is OFF.
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "usb.h"

#define LED_CONFIG	    (DDRC |= (1 << 7))
#define LED_OFF		      (PORTC &= ~(1 << 7))
#define LED_ON 		      (PORTC |= (1 << 7))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

static const int8_t PROGMEM circle[] =
{
  16, -1,
  15, -4,
  14, -7,
  13, -9,
  11, -11,
  9, -13,
  7, -14,
  4, -15,
  1, -16,
  -1, -16,
  -4, -15,
  -7, -14,
  -9, -13,
  -11, -11,
  -13, -9,
  -14, -7,
  -15, -4,
  -16, -1,
  -16, 1,
  -15, 4,
  -14, 7,
  -13, 9,
  -11, 11,
  -9, 13,
  -7, 14,
  -4, 15,
  -1, 16,
  1, 16,
  4, 15,
  7, 14,
  9, 13,
  11, 11,
  13, 9,
  14, 7,
  15, 4,
  16, 1
};

int main(void)
{
  int8_t x, y;
  const int8_t *p;
  uint8_t i;

  // set for 16 MHz clock
  CPU_PRESCALE(0);
  LED_CONFIG;
  LED_OFF;

  // Initialize the USB, and then wait for the host to set configuration.
  // If the board is powered without a PC connected to the USB port,
  // this will wait forever.
  usb_init();
  while (! usb_configured()) /* wait */ ;

  // Wait an extra second for the PC's operating system to load drivers
  // and do whatever it does to actually be ready for input
  for (i = 0; i < 10; i++)
    _delay_ms(100);

  while (1)
  {
    for (i = 0; i < 10; i++)
      _delay_ms(100);

    LED_ON;  // turn the LED on while moving the mouse
    p = circle;

    for (i = 0; i < 36; i++)
    {
      x = pgm_read_byte(p++);
      y = pgm_read_byte(p++);
      move_mouse(x, y);
      _delay_ms(50);
    }

    LED_OFF;

    for (i = 0; i < 50; i++)
      _delay_ms(100);
  }
}
