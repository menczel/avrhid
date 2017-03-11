/*
  hid_joy.h
  Copyright (C) 2017 Laszlo Menczel

  This is free software distributed under the terms of the MIT license
  (see the file LICENSE or https://opensource.org/licenses/MIT).
 
*/

#if ! defined HID_JOY_H
#define HID_JOY_H

void copy_report_to_buf(void);

#define DATA_EP_BUF_SIZE     64
#define AVR_DATA_BUF_CODE    0x30   // Atmega code for 64 bytes  
#define IF_PROTOCOL          0
#define IF_SUBCLASS          0

#define HID_DESC_SIZE        86

typedef struct
{
  uint16_t buttons;
  int16_t  x;
  int16_t  y;
  uint8_t  sc1;
  uint8_t  sc2;
  uint8_t  hat;
} joy_state_t;

extern joy_state_t joy_state;
extern const uint8_t hid_report_desc[] PROGMEM;

#endif /* HID_JOY_H */
