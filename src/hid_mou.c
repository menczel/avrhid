/*
  hid_mouse.c
  Copyright (c) 2011, Peter Barrett
  Modifications: Copyright (C) 2017 Laszlo Menczel

  This is free software distributed under the terms of the MIT license
  (see the file LICENSE or https://opensource.org/licenses/MIT).
 
  ======================================================================

  Implements a standard mouse with three buttons and a wheel.
*/

#include "usb.h"

// report descriptor size 51 bytes

const uint8_t PROGMEM hid_report_desc[] =
{
	0x05, 0x01,			// Usage Page (Generic Desktop)
	0x09, 0x02,			// Usage (Mouse)
	0xa1, 0x01,			// Collection (Application)

  // Buton status
	0x05, 0x09,			//   Usage Page (Button)
	0x19, 0x01,			//   Usage Minimum (Button #1)
	0x29, 0x03,			//   Usage Maximum (Button #3)
	0x15, 0x00,			//   Logical Minimum (0)
	0x25, 0x01,			//   Logical Maximum (1)
	0x95, 0x03,			//   Report Count (3)
	0x75, 0x01,			//   Report Size (1)
	0x81, 0x02,			//   Input (Data, Variable, Absolute)

  // Padding to byte boundary
	0x95, 0x01,			//   Report Count (1)
	0x75, 0x05,			//   Report Size (5)
	0x81, 0x03,			//   Input (Constant)

  // X and Y axis
	0x05, 0x01,			//   Usage Page (Generic Desktop)
	0x09, 0x30,			//   Usage (X)
	0x09, 0x31,			//   Usage (Y)
	0x15, 0x81,			//   Logical Minimum (-127)
	0x25, 0x7f,			//   Logical Maximum (127)
	0x75, 0x08,			//   Report Size (8),
	0x95, 0x02,			//   Report Count (2),
	0x81, 0x06,			//   Input (Data, Variable, Relative)

  // Wheel rotation
	0x09, 0x38,			//   Usage (Wheel)
	0x95, 0x01,			//   Report Count (1),
	0x81, 0x06,			//   Input (Data, Variable, Relative)
	0xC0				// End Collection
};

mouse_state_t mouse_state;

//======================================================================

void
copy_report_to_buf(void)
{
  uint8_t byte;

  byte = 0;

  if (mouse_state.but_left)
    byte |= 0x01;
  else
    byte &= 0xfe;

  if (mouse_state.but_right)
    byte |= 0x02;
  else
    byte &= 0xfd;
    
  if (mouse_state.but_mid)
    byte |= 0x04;
  else
    byte &= 0xfb;

  UEDATX = byte;
  UEDATX = mouse_state.x;
  UEDATX = mouse_state.y;
  UEDATX = mouse_state.wheel;
}

//======================================================================
// Function for testing the device
//======================================================================

void
move_mouse(int8_t x, int8_t y)
{
  uint8_t intr_state, timeout;

  if (! usb_configured())
    return;

  if (x == -128)
    x = -127;

  if (y == -128)
    y = -127;

  intr_state = SREG;
  cli();

  UENUM = ENDPOINT_NUM;
  timeout = UDFNUML + 50;

  while (1)
  {
    // are we ready to transmit?
    if (UEINTX & (1<<RWAL))
      break;

    SREG = intr_state;

    // has the USB gone offline?
    if (! usb_configured())
      return;

    // have we waited too long?
    if (UDFNUML == timeout)
      return;

    // get ready to try checking again
    intr_state = SREG;
    cli();
    UENUM = ENDPOINT_NUM;
  }

  mouse_state.x = x;
  mouse_state.y = y;
  copy_report_to_buf();

  UEINTX = 0x3A;
  SREG = intr_state;
  return;
}
