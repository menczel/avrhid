/*
  hid_kbd.h
  Copyright (C) 2017 Laszlo Menczel

  This is free software distributed under the terms of the MIT license
  (see the file LICENSE or https://opensource.org/licenses/MIT).
*/

#if ! defined HID_MOUSE_H
#define HID_MOUSE_H

void copy_report_to_buf(void);

// Functions for testing the device
extern void move_mouse(int8_t x, int8_t y);

#define DATA_EP_BUF_SIZE     8
#define AVR_DATA_BUF_CODE    0      // Atmega code for 8 bytes  
#define IF_PROTOCOL          2
#define IF_SUBCLASS          1

#define HID_DESC_SIZE        51

typedef struct
{
  uint8_t but_left;
  uint8_t but_right;
  uint8_t but_mid;
  int8_t  wheel;
  int8_t  x;
  int8_t  y;
} mouse_state_t;

extern mouse_state_t mouse_state;
extern const uint8_t PROGMEM hid_report_desc[];

#endif /* HID_MOUSE_H */
