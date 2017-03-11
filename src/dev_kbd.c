/*
  main.c

  Keyboard example for Teensy USB Development Board
  http://www.pjrc.com/teensy/usb_keyboard.html
  Copyright (c) 2008 PJRC.COM, LLC
   
  Modified for implementing HID devices on Arduino boards
  Implements: keyboard, mouse and joystick
  Modifications Copyright (C) 2017 Laszlo Menczel

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

  ======================================================================

  This is a simple program to test the HID keyboard code. If it works
  then it should be replaced by the real device handler that polls the
  I/O ports, updates the state structure of the keyboard and sends the
  keyboard report.

  This program sends keypresses if one of the I/O ports in the set
  D0 - D7 and B0 - B7 is briefly connected to the ground (use a wire).
  If no keypress is simulated the program sends a space character every
  5 seconds.
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usb.h"

#define LED_CONFIG      (DDRD |= (1 << 6))
#define LED_ON          (PORTD &= ~(1 << 6))
#define LED_OFF         (PORTD |= (1 << 6))
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

#define IDLE_TIMEOUT    5 // send something automatically every 5 seconds

//======================================================================

uint8_t number_keys[10]=
{KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9};

uint16_t idle_count = 0;

//======================================================================

int
main(void)
{
  uint8_t b, d, mask, i, got_event;
  uint8_t b_prev=0xFF, d_prev=0xFF;

  // set for 16 MHz clock
  CPU_PRESCALE(0);

  // Configure all port B and port D pins as inputs with pullup resistors.
  DDRD = 0x00;
  DDRB = 0x00;
  PORTB = 0xFF;
  PORTD = 0xFF;

  // Initialize the USB, and then wait for the host to set configuration.
  // If the board is powered without a PC connected to the USB port,
  // this will wait forever.
  usb_init();
  while (! usb_configured())
    ; /* wait */

  // Wait an extra second for the PC's operating system to load drivers
  // and do whatever it does to actually be ready for input
  for (i = 0; i < 10; i++)
    _delay_ms(100);

  // Configure timer 0 to generate a timer overflow interrupt every
  // 256*1024 clock cycles, or approx 61 Hz when using 16 MHz clock
  // This demonstrates how to use interrupts to implement a simple
  // inactivity timeout.
  TCCR0A = 0x00;
  TCCR0B = 0x05;
  TIMSK0 = (1 << TOIE0);

  while (1)
  {
    // read all port B and port D pins
    b = PINB;
    d = PIND;

    // check if any pins are low, but were high previously
    mask = 1;
    got_event = 0;

    for (i = 0; i < 8; i++)
    {
      if (((b & mask) == 0) && (b_prev & mask) != 0)
      {
        press_key(KEY_B, KEY_SHIFT);
        press_key(number_keys[i], 0);
        got_event = 1;
      }

      if (((d & mask) == 0) && (d_prev & mask) != 0)
      {
        press_key(KEY_D, KEY_SHIFT);
        press_key(number_keys[i], 0);
        got_event = 1;
      }

      mask = mask  <<  1;
    }

    // if any keypresses were detected, reset the idle counter
    if (got_event)
    {
      // variables shared with interrupt routines must be
      // accessed carefully so the interrupt routine doesn't
      // try to use the variable in the middle of our access
      cli();
      idle_count = 0;
      sei();
    }

    // now the current pins will be the previous, and
    // wait a short delay so we're not highly sensitive
    // to mechanical "bounce".
    b_prev = b;
    d_prev = d;
    _delay_ms(5);
  }
}

//======================================================================
// This interrupt routine is run approx 61 times per second.
// A very simple inactivity timeout is implemented, where we
// will send a space character every IDLE_TIMEOUT seconds.

ISR(TIMER0_OVF_vect)
{
  idle_count++;

  if (idle_count > 61 * IDLE_TIMEOUT)
  {
    idle_count = 0;
    press_key(KEY_SPACE, 0);
  }
}
