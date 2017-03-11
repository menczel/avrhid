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
  I/O ports and updates the state structure of the keyboard.

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

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

enum { DIR_LEFT, DIR_RIGHT };
enum { DIR_DOWN, DIR_UP };

#define SMIN       -511
#define SMAX       511
#define UMAX       255

#define INCR        10

int main(void)
{
  uint8_t dir[2] = { DIR_RIGHT, DIR_UP };
  uint8_t i, cycle, portval, mask;

  // set for 16 MHz clock
  CPU_PRESCALE(0);

  // Configure all port B and port D pins as inputs with pullup resistors.
  DDRD = 0x00;
  PORTD = 0xFF;
  
  // Initialize the USB, and then wait for the host to set configuration.
  // If the board is powered without a PC connected to the USB port,
  // this will wait forever.
  usb_init();
  while (! usb_configured())
    ;

  // Wait 10 seconds for the PC's operating system to load drivers
  // and for the user to start the joystick test program
  for (i = 0; i < 100; i++)
    _delay_ms(100);

  cycle = 0;

  while (1)
  {
    // simulate horizontal move
    if (dir[0] == DIR_RIGHT)
    {
      if (joy_state.x < SMAX - INCR)
        joy_state.x += INCR;
      else
      {
        joy_state.x = SMAX;
        dir[0] = DIR_LEFT;
      }
    }
    else  // currently moving to the left
    {
      if (joy_state.x > SMIN + INCR)
        joy_state.x -= INCR;
      else
      {
        joy_state.x = SMIN;
        dir[0] = DIR_RIGHT;
      }
    }

    // simulate vertical move
    if (dir[1] == DIR_DOWN)
    {
      if (joy_state.y < SMAX - INCR)
        joy_state.y += INCR;
      else
      {
        joy_state.y = SMAX;
        dir[1] = DIR_UP;
      }
    }
    else  // currently moving up
    {
      if (joy_state.y > SMIN + INCR)
        joy_state.y -= INCR;
      else
      {
        joy_state.y = SMIN;
        dir[1] = DIR_DOWN;
      }
    }

    // simulate throttle and rudder
    if (joy_state.sc1 > UMAX - INCR)
      joy_state.sc1 = 0;
    else
      joy_state.sc1 += INCR;
    
    if (joy_state.sc2 > UMAX - INCR)
      joy_state.sc2 = 0;
    else
      joy_state.sc2 += INCR;

    // simulate HAT switch
    cycle++;
    if (cycle == 10)
    {
      cycle = 0;
      joy_state.hat++;
      if (joy_state.hat == 8)
        joy_state.hat = 0;
    }

    // test Port D for simulated button press (grounding the pin)
    portval = PIND;
    for (i = 0, mask = 1; i < 8; i++, mask = mask << 1)
    {
      if (portval & mask)                         // not pressed
        joy_state.buttons &= ~((uint16_t) mask);
      else
        joy_state.buttons |= (uint16_t) mask;
    }

    send_hid_report();
    _delay_ms(100);
  }
}
