/*
  usb.h

  Keyboard example for Teensy USB Development Board
  http://www.pjrc.com/teensy/usb_keyboard.html
  Copyright (c) 2008 PJRC.COM, LLC
   
  Modified for implementing HID devices on Arduino boards
  Implements: keyboard, mouse and joystick
  Modifications Copyright (C) 2017 Laszlo Menczel

  This is free software distributed under the terms of the MIT license
  (see the file LICENSE or https://opensource.org/licenses/MIT).
*/

#if ! defined USB_H
#define USB_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

//======================================================================
// check for multiple HID types

#if defined HID_KEYBOARD && (defined HID_MOUSE || defined HID_JOYSTICK)
  #error Multiple definitions of HID type in usb.h
#endif

#if defined HID_MOUSE && (defined HID_KEYBOARD || defined HID_JOYSTICK)
  #error Multiple definitions of HID type in usb.h
#endif

#if defined HID_JOYSTICK && (defined HID_KEYBOARD || defined HID_MOUSE)
  #error Multiple definitions of HID type in usb.h
#endif

//======================================================================

extern void usb_init(void);
extern uint8_t usb_configured(void);
extern int8_t send_hid_report(void);

//======================================================================
// Get device specific stuff

#if defined HID_KEYBOARD
  #include "hid_kbd.h"
#endif

#if defined HID_MOUSE
  #include "hid_mou.h"
#endif

#if defined HID_JOYSTICK
  #include "hid_joy.h"
#endif

//======================================================================

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

#define ENDPOINT_NUM           3

//======================================================================
// Everything below this point is only intended for usb.c
//======================================================================

#if defined PRIVATE_INCLUDE

#define EP_TYPE_CONTROL         0x00
#define EP_TYPE_INTERRUPT_IN    0xC1

#define EP_SINGLE_BUFFER        0x02
#define EP_DOUBLE_BUFFER        0x06

// standard control endpoint request types
#define GET_STATUS              0
#define CLEAR_FEATURE           1
#define SET_FEATURE             3
#define SET_ADDRESS             5
#define GET_DESCRIPTOR          6
#define GET_CONFIGURATION       8
#define SET_CONFIGURATION       9
#define GET_INTERFACE           10
#define SET_INTERFACE           11

// HID (human interface device)
#define HID_GET_REPORT          1
#define HID_GET_IDLE            2
#define HID_GET_PROTOCOL        3
#define HID_SET_REPORT          9
#define HID_SET_IDLE            10
#define HID_SET_PROTOCOL        11

#endif /* PRIVATE_INCLUDE */

#endif /* USB_H */
