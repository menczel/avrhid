/*
  hid_kbd.c
  Copyright (c) 2011, Peter Barrett
  Modifications: Copyright (C) 2017 Laszlo Menczel

  This is free software distributed under the terms of the MIT license
  (see the file LICENSE or https://opensource.org/licenses/MIT).
 
  ======================================================================

  Implements a standard keyboard with 101 keys (including 8 modifiers).
*/

#include "usb.h"

// report descriptor size 45 bytes

const uint8_t PROGMEM hid_report_desc[] =
{
  0x05, 0x01,         // USAGE_PAGE (Generic Desktop)	// 47
  0x09, 0x06,         // USAGE (Keyboard)
  0xa1, 0x01,         // COLLECTION (Application)

  // status of modifier keys (one bit / key)
  0x05, 0x07,         //   USAGE_PAGE (Key codes)
	0x19, 0xe0,         //   USAGE_MINIMUM (Keyboard LeftControl)
  0x29, 0xe7,         //   USAGE_MAXIMUM (Keyboard Right GUI)
  0x15, 0x00,         //   LOGICAL_MINIMUM (0)
  0x25, 0x01,         //   LOGICAL_MAXIMUM (1)
  0x75, 0x01,         //   REPORT_SIZE (1)
	0x95, 0x08,         //   REPORT_COUNT (8)
  0x81, 0x02,         //   INPUT (Data,Var,Abs)

  // Marked 'reserved' is some descriptor samples.
  // Maybe padding to make the report 8 bytes?
  0x75, 0x08,         //   REPORT_SIZE (8)
  0x95, 0x01,         //   REPORT_COUNT (1)
  0x81, 0x03,         //   INPUT (Const,Var,Abs)

  // index of up to six keys pressed (out of 101 keys)
  0x05, 0x07,         //   USAGE_PAGE (Key codes)
	0x19, 0x00,         //   USAGE_MINIMUM (0)
  0x29, 0x65,         //   USAGE_MAXIMUM (101)
  0x15, 0x00,         //   LOGICAL_MINIMUM (0)
  0x25, 0x65,         //   LOGICAL_MAXIMUM (101)
  0x75, 0x08,         //   REPORT_SIZE (8)
	0x95, 0x06,         //   REPORT_COUNT (6)
  0x81, 0x00,         //   INPUT (Data,Array,Abs)

  0xc0,               // END OF COLLECTION 
};

kbd_state_t kbd_state = { 0, 0, { 0, 0, 0, 0, 0, 0 } };

//======================================================================

void
copy_report_to_buf(void)
{
  int i;

  UEDATX = kbd_state.modkeys;
  UEDATX = 0;
  for (i = 0; i < 6; i++)
    UEDATX = kbd_state.keys[i];
}

//======================================================================
// Functions for testing the device
//======================================================================

void
press_key(uint8_t key, uint8_t modifier)
{
  int8_t res;

  kbd_state.modkeys = modifier;
  kbd_state.keys[0] = key;
  res = send_hid_report();

  if (res)
    return;

  kbd_state.modkeys = 0;
  kbd_state.keys[0] = 0;
  send_hid_report();
}
