/*
  hid_kbd.h
  Copyright (C) 2017 Laszlo Menczel

  Permission to use, copy, modify, and/or distribute this software for  
  any purpose with or without fee is hereby granted, provided that the  
  above copyright notice and this permission notice appear in all copies.  
  
  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL  
  WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED  
  WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR  
  BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES  
  OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  
  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS  
  SOFTWARE.  
*/

#if ! defined HID_KBD_H
#define HID_KBD_H

extern void copy_report_to_buf(void);

// Functions for testing the device
extern void press_key(uint8_t key, uint8_t modifier);

#define DATA_EP_BUF_SIZE     8
#define AVR_DATA_BUF_CODE    0      // Atmega code for 8 bytes  
#define IF_PROTOCOL          1
#define IF_SUBCLASS          1

#define HID_DESC_SIZE        45

// structure to store the status of keys, modifiers and LEDs
// modkeys = modifier keys currently pressed
// 1=left ctrl,    2=left shift,   4=left alt,    8=left gui
// 16=right ctrl, 32=right shift, 64=right alt, 128=right gui
// leds = status of keyboard LEDs
// keys = which keys are pressed, up to 6 keys may be down at once
typedef struct
{
  uint8_t modkeys;
  uint8_t leds;
  uint8_t keys[6];
} kbd_state_t;

extern kbd_state_t kbd_state;
extern const uint8_t PROGMEM hid_report_desc[];

#include "keycodes.h"

#endif /* HID_KBD_H */
