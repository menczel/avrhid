/*
  hid_joy.c
  Copyright (c) 2011, Peter Barrett
  Modifications: Copyright (C) 2017 Laszlo Menczel

  This is free software distributed under the terms of the MIT license
  (see the file LICENSE or https://opensource.org/licenses/MIT).
 
  ======================================================================

  Implements a joystick with 2 axis, 2 simulation controls (throttle and
  rudder), one HAT switch and 16 buttons. The axis have -511 to 511 range
  (10 bit resolution). The controls have 0 to 255 range.
*/

#include "usb.h"

// report descriptor size 86 bytes

const uint8_t hid_report_desc[] PROGMEM =
{
  0x05, 0x01,            // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,            // USAGE (Joystick)
  0xa1, 0x01,            // COLLECTION (Application)

  // Button status
	0x05, 0x09,			       //   Usage Page (Button)
	0x19, 0x01,			       //   Usage Minimum (Button #1)
	0x29, 0x10,			       //   Usage Maximum (Button #16)
	0x15, 0x00,			       //   Logical Minimum (0)
	0x25, 0x01,			       //   Logical Maximum (1)
	0x75, 0x01,			       //   Report Size (1)
	0x95, 0x10,			       //   Report Count (16)
	0x81, 0x02,			       //   Input (Data, Variable, Absolute)

  // Axis 
  0x05, 0x01,            //   USAGE_PAGE (Generic Desktop)
  0x09, 0x30,            //   USAGE (x)
  0x09, 0x31,            //   USAGE (y)
  0x16, 0x01, 0xfe,      //   LOGICAL_MINIMUM (-511)
  0x26, 0xff, 0x01,      //   LOGICAL_MAXIMUM (511)
  0x75, 0x10,            //   REPORT_SIZE (16)
  0x95, 0x02,            //   REPORT_COUNT (2)
  0x81, 0x02,            //   INPUT (Data, Variable, Absolute)

  // Throttle and rudder
  0x05, 0x02,            //   USAGE_PAGE (Simulation Controls)
  0x15, 0x00,            //   LOGICAL_MINIMUM (0)
  0x26, 0xff, 0x00,      //   LOGICAL_MAXIMUM (255)
  0xA1, 0x00,            //   COLLECTION (Physical)
  0x09, 0xBB,            //     USAGE (Throttle)
  0x09, 0xBA,            //     USAGE (Steering)
  0x75, 0x08,            //     REPORT_SIZE (8)
  0x95, 0x02,            //     REPORT_COUNT (2)
  0x81, 0x02,            //     INPUT (Data, Variable, Absolute)
  0xc0,                  //   END_COLLECTION

  // HAT switch
  0x05, 0x01,            //   USAGE_PAGE (Generic Desktop)
  0x09, 0x39,            //   USAGE (Hat switch)
  0x15, 0x00,            //   LOGICAL_MINIMUM (0)
  0x25, 0x07,            //   LOGICAL_MAXIMUM (7)
  0x35, 0x00,            //   PHYSICAL_MINIMUM (0)
  0x46, 0x3B, 0x01,      //   PHYSICAL_MAXIMUM (315)
  0x75, 0x04,            //   REPORT_SIZE (4)
  0x95, 0x01,            //   REPORT_COUNT (1)
  0x81, 0x02,            //   INPUT (Data, Variable, Absolute)
	0x95, 0x01,			       //   Padding to byte boundary (Report Count 1)  
	0x75, 0x04,			       //   Report Size (4)
	0x81, 0x03,			       //   Input (Constant)

  0xc0                   // END_COLLECTION
};

joy_state_t joy_state = { 0, 0, 0, 0, 0 };

//======================================================================

void
copy_report_to_buf(void)
{
  UEDATX = LSB(joy_state.buttons);
  UEDATX = MSB(joy_state.buttons);

  UEDATX = LSB(joy_state.x);
  UEDATX = MSB(joy_state.x);

  UEDATX = LSB(joy_state.y);
  UEDATX = MSB(joy_state.y);

  UEDATX = joy_state.sc1;
  UEDATX = joy_state.sc2;

  UEDATX = joy_state.hat;
}
